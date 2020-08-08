/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#include "imstkColorFunction.h"
#include "imstkGeometryUtilities.h"
#include "imstkMeshIO.h"
#include "imstkPointSet.h"
#include "imstkScene.h"
#include "imstkSimulationManager.h"
#include "imstkSPHModel.h"
#include "imstkSPHObject.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"

#include <vtkBooleanOperationPolyDataFilter.h>
#include <vtkCenterOfMass.h>
#include <vtkCleanPolyData.h>
#include <vtkPolyData.h>
#include <vtkSelectEnclosedPoints.h>
#include <vtkTriangleFilter.h>

#include "imstkSPHBoundaryConditions.h"
#include "imstkSPHHemorrhage.h"


using namespace imstk;


///
/// \brief Generate wall points for pipe flow
///
std::shared_ptr<SurfaceMesh>
generateWallFluidPoints(const double particleRadius, std::shared_ptr<SurfaceMesh>& surfMesh, std::shared_ptr<SurfaceMesh>& surfMeshExpanded)
{
    // subtract original mesh from expanded mesh so we can get wall mesh
    auto intersectionPolyDataFilter = vtkSmartPointer<vtkBooleanOperationPolyDataFilter>::New();
    intersectionPolyDataFilter->SetOperationToDifference();

    auto vtkPolySurfMesh = GeometryUtils::convertSurfaceMeshToVtkPolyData(surfMesh);
    auto vtkPolySurfMeshExpanded = GeometryUtils::convertSurfaceMeshToVtkPolyData(surfMeshExpanded);

    vtkSmartPointer<vtkTriangleFilter> tri1 =
      vtkSmartPointer<vtkTriangleFilter>::New();
    tri1->SetInputData(vtkPolySurfMesh);
    tri1->Update();
    vtkSmartPointer<vtkCleanPolyData> clean1 =
      vtkSmartPointer<vtkCleanPolyData>::New();
    clean1->SetInputConnection(tri1->GetOutputPort());
    clean1->Update();
    auto input1 = clean1->GetOutput();

    vtkSmartPointer<vtkTriangleFilter> tri2 =
      vtkSmartPointer<vtkTriangleFilter>::New();
    tri2->SetInputData(vtkPolySurfMeshExpanded);
    tri2->Update();
    vtkSmartPointer<vtkCleanPolyData> clean2 =
      vtkSmartPointer<vtkCleanPolyData>::New();
    clean2->SetInputConnection(tri2->GetOutputPort());
    clean2->Update();
    auto input2 = clean2->GetOutput();

    intersectionPolyDataFilter->SetInputData(0, input2);
    intersectionPolyDataFilter->SetInputData(1, input1);
    intersectionPolyDataFilter->Update();

    auto outputPolyData = intersectionPolyDataFilter->GetOutput();

    std::shared_ptr<SurfaceMesh> subtractedMesh = std::move(GeometryUtils::convertVtkPolyDataToSurfaceMesh(outputPolyData));

    return subtractedMesh;
}

std::shared_ptr<SPHObject>
generateFluid(const std::shared_ptr<Scene>& scene, const double particleRadius)
{
    StdVectorOfVec3d particles;
    auto sphModel = std::make_shared<SPHModel>();
    double speedOfSound = 100;
    double restDensity = 1;
    if (SCENE_ID == 1)
    {
      // pipe flow
      auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(MeshIO::read(iMSTK_DATA_ROOT "/cylinder/cylinder_small.stl"));
      auto surfMeshShell = std::dynamic_pointer_cast<SurfaceMesh>(MeshIO::read(iMSTK_DATA_ROOT "/cylinder/cylinder_small_shell.stl"));
      auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(MeshIO::read(iMSTK_DATA_ROOT "/cylinder/cylinder_small.vtk"));

      //surfMesh->rotate(Vec3d(1, 0, 0), PI / 5, Geometry::TransformType::ConcatenateToTransform);
      //surfMeshShell->rotate(Vec3d(1, 0, 0), PI / 5, Geometry::TransformType::ConcatenateToTransform);
      //tetMesh->rotate(Vec3d(1, 0, 0), PI / 5, Geometry::TransformType::ConcatenateToTransform);

      // set tetrahedral mesh used when writing VTUs
      sphModel->setGeometryMesh(tetMesh);

      // fill grid with points
      Vec3d aabbMin, aabbMax;
      surfMeshShell->computeBoundingBox(aabbMin, aabbMax, 1.);
      const double length = std::abs(aabbMax.x() - aabbMin.x());
      const double width = std::abs(aabbMax.y() - aabbMin.y());
      const double depth = std::abs(aabbMax.z() - aabbMin.z());
      const auto spacing = 2.2 * particleRadius;
      const auto wallSpacing = 1.9 * particleRadius;
      const auto nx = static_cast<size_t>(length / spacing);
      const auto ny = static_cast<size_t>(width / spacing);
      const auto nz = static_cast<size_t>(depth / spacing);
      const auto nx_wall = static_cast<size_t>(length / wallSpacing);
      const auto ny_wall = static_cast<size_t>(width / wallSpacing);
      const auto nz_wall = static_cast<size_t>(depth / wallSpacing);
      auto uniformMesh = std::dynamic_pointer_cast<PointSet>(GeometryUtils::createUniformMesh(aabbMin, aabbMax, nx, ny, nz));
      auto uniformMesh_wall = std::dynamic_pointer_cast<PointSet>(GeometryUtils::createUniformMesh(aabbMin, aabbMax, nx_wall, ny_wall, nz_wall));

      auto enclosedFluidPoints = GeometryUtils::getEnclosedPoints(surfMesh, uniformMesh, false);
      particles = enclosedFluidPoints->getInitialVertexPositions();
      auto enclosedWallPoints = GeometryUtils::getEnclosedPoints(surfMeshShell, uniformMesh_wall, false);
      StdVectorOfVec3d wallParticles = enclosedWallPoints->getInitialVertexPositions();

      // set up inlet boundary conditions
      double inletFlowRate = 10;
      double inletRadius = 0.6;
      Vec3d inletCenterPoint = Vec3d(-2.5, 0.0, 0);
      Vec3d inletMinCoord = inletCenterPoint - Vec3d(0.1, inletRadius, inletRadius);
      Vec3d inletMaxCoord = inletCenterPoint + Vec3d(0.5, inletRadius, inletRadius);
      Vec3d inletNormal(-1, 0, 0);
      std::pair<Vec3d, Vec3d> inletCoords = std::make_pair(inletMinCoord, inletMaxCoord);

      // set up outlet boundary conditions
      double outletRadius = 0.5;
      Vec3d outletCenterPoint = Vec3d(2.5, 0.0, 0.0);
      Vec3d outletMinCoord = outletCenterPoint - Vec3d(0.5, outletRadius, outletRadius);
      Vec3d outletMaxCoord = outletCenterPoint + Vec3d(0, outletRadius, outletRadius);
      StdVectorOfVec3d outletNormals{ Vec3d(1, 0, 0) };
      std::vector<std::pair<Vec3d, Vec3d>> outletCoords{ std::make_pair(outletMinCoord, outletMaxCoord) };

      auto sphBoundaryConditions = std::make_shared<SPHBoundaryConditions>(inletCoords, outletCoords, inletNormal, outletNormals, inletRadius, inletCenterPoint, inletFlowRate, particles, wallParticles);
      sphModel->setBoundaryConditions(sphBoundaryConditions);
  }

  else if (SCENE_ID == 2)
  {
      // half torus flow
      auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(MeshIO::read(iMSTK_DATA_ROOT "/torus/torus.stl"));
      auto surfMeshShell = std::dynamic_pointer_cast<SurfaceMesh>(MeshIO::read(iMSTK_DATA_ROOT "/torus/torus_shell.stl"));
      auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(MeshIO::read(iMSTK_DATA_ROOT "/torus/torus.vtk"));

      // set tetrahedral mesh used when writing VTUs
      sphModel->setGeometryMesh(tetMesh);

      // fill grid with points
      Vec3d aabbMin, aabbMax;
      surfMeshShell->computeBoundingBox(aabbMin, aabbMax, 1.);
      const double length = std::abs(aabbMax.x() - aabbMin.x());
      const double width = std::abs(aabbMax.y() - aabbMin.y());
      const double depth = std::abs(aabbMax.z() - aabbMin.z());
      const auto spacing = 2.2 * particleRadius;
      const auto wallSpacing = 1.9 * particleRadius;
      const auto nx = static_cast<size_t>(length / spacing);
      const auto ny = static_cast<size_t>(width / spacing);
      const auto nz = static_cast<size_t>(depth / spacing);
      const auto nx_wall = static_cast<size_t>(length / wallSpacing);
      const auto ny_wall = static_cast<size_t>(width / wallSpacing);
      const auto nz_wall = static_cast<size_t>(depth / wallSpacing);
      auto uniformMesh = std::dynamic_pointer_cast<PointSet>(GeometryUtils::createUniformMesh(aabbMin, aabbMax, nx, ny, nz));
      auto uniformMesh_wall = std::dynamic_pointer_cast<PointSet>(GeometryUtils::createUniformMesh(aabbMin, aabbMax, nx_wall, ny_wall, nz_wall));

      auto enclosedFluidPoints = GeometryUtils::getEnclosedPoints(surfMesh, uniformMesh, false);
      particles = enclosedFluidPoints->getInitialVertexPositions();
      auto enclosedWallPoints = GeometryUtils::getEnclosedPoints(surfMeshShell, uniformMesh_wall, false);
      StdVectorOfVec3d wallParticles = enclosedWallPoints->getInitialVertexPositions();

      // set up inlet boundary conditions
      double inletFlowRate = 5;
      double inletRadius = 0.4;
      Vec3d inletCenterPoint = Vec3d(-0.45, 1.5, 0.0);
      Vec3d inletMinCoord = inletCenterPoint - Vec3d(0.1, inletRadius, inletRadius);
      Vec3d inletMaxCoord = inletCenterPoint + Vec3d(0.5, inletRadius, inletRadius);
      Vec3d inletNormal(-1, 0, 0);
      std::pair<Vec3d, Vec3d> inletCoords = std::make_pair(inletMinCoord, inletMaxCoord);

      // set up outlet boundary conditions
      double outletRadius = 0.4;
      Vec3d outletCenterPoint = Vec3d(-0.45, -1.5, 0.0);
      Vec3d outletMinCoord = outletCenterPoint - Vec3d(0.0, outletRadius, outletRadius);
      Vec3d outletMaxCoord = outletCenterPoint + Vec3d(0.5, outletRadius, outletRadius);
      StdVectorOfVec3d outletNormal{ Vec3d(1, 0, 0) };
      std::vector<std::pair<Vec3d, Vec3d>> outletCoords{ std::make_pair(outletMinCoord, outletMaxCoord) };

      auto sphBoundaryConditions = std::make_shared<SPHBoundaryConditions>(inletCoords, outletCoords, inletNormal,
        outletNormal, inletRadius, inletCenterPoint, inletFlowRate, particles, wallParticles);
      sphModel->setBoundaryConditions(sphBoundaryConditions);

      speedOfSound = 300;
  }

  else if (SCENE_ID == 3)
  {
      // bifurcation flow
      auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(MeshIO::read(iMSTK_DATA_ROOT "/bifurcation/bifurcation_small.stl"));
      auto surfMeshShell = std::dynamic_pointer_cast<SurfaceMesh>(MeshIO::read(iMSTK_DATA_ROOT "/bifurcation/bifurcation_small_shell.stl"));
      auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(MeshIO::read(iMSTK_DATA_ROOT "/bifurcation/bifurcation_small.vtk"));

      // set tetrahedral mesh used when writing VTUs
      sphModel->setGeometryMesh(tetMesh);

      // fill grid with points
      Vec3d aabbMin, aabbMax;
      surfMeshShell->computeBoundingBox(aabbMin, aabbMax, 1.);
      const double length = std::abs(aabbMax.x() - aabbMin.x());
      const double width = std::abs(aabbMax.y() - aabbMin.y());
      const double depth = std::abs(aabbMax.z() - aabbMin.z());
      const auto spacing = 2.2 * particleRadius;
      const auto wallSpacing = 2.0 * particleRadius;
      const auto nx = static_cast<size_t>(length / spacing);
      const auto ny = static_cast<size_t>(width / spacing);
      const auto nz = static_cast<size_t>(depth / spacing);
      const auto nx_wall = static_cast<size_t>(length / wallSpacing);
      const auto ny_wall = static_cast<size_t>(width / wallSpacing);
      const auto nz_wall = static_cast<size_t>(depth / wallSpacing);
      auto uniformMesh = std::dynamic_pointer_cast<PointSet>(GeometryUtils::createUniformMesh(aabbMin, aabbMax, nx, ny, nz));
      auto uniformMesh_wall = std::dynamic_pointer_cast<PointSet>(GeometryUtils::createUniformMesh(aabbMin, aabbMax, nx_wall, ny_wall, nz_wall));

      auto enclosedFluidPoints = GeometryUtils::getEnclosedPoints(surfMesh, uniformMesh, false);
      particles = enclosedFluidPoints->getInitialVertexPositions();
      auto enclosedWallPoints = GeometryUtils::getEnclosedPoints(surfMeshShell, uniformMesh_wall, false);
      StdVectorOfVec3d wallParticles = enclosedWallPoints->getInitialVertexPositions();
  
      // set up inlet boundary condition
      double inletFlowRate = 5;
      double inletRadius = 0.35;
      Vec3d inletCenterPoint = Vec3d(-1.65, 0.0, 0.0);
      Vec3d inletMinCoord = inletCenterPoint - Vec3d(0.1, inletRadius, inletRadius);
      Vec3d inletMaxCoord = inletCenterPoint + Vec3d(0.3, inletRadius, inletRadius);
      Vec3d inletNormal(-1, 0, 0);
      std::pair<Vec3d, Vec3d> inletCoords = std::make_pair(inletMinCoord, inletMaxCoord);

      // set up outlet boundary conditions - in this case, we have two outlets
      double outletRadius = 0.5;
      Vec3d outletCenterPoint1 = Vec3d(1.82, 1.45, 0);
      Vec3d outletCenterPoint2 = Vec3d(1.82, -1.452, 0);
      Vec3d outletMinCoord1 = outletCenterPoint1 - Vec3d(0.3, outletRadius, outletRadius);
      Vec3d outletMaxCoord1 = outletCenterPoint1 + Vec3d(0.0, outletRadius, outletRadius);
      Vec3d outletMinCoord2 = outletCenterPoint2 - Vec3d(0.3, outletRadius, outletRadius);
      Vec3d outletMaxCoord2 = outletCenterPoint2 + Vec3d(0, outletRadius, outletRadius);
      StdVectorOfVec3d outletNormals{ Vec3d(1, 0, 0), Vec3d(1, 0, 0) };
      std::vector<std::pair<Vec3d, Vec3d>> outletCoords{ std::make_pair(outletMinCoord1, outletMaxCoord1), std::make_pair(outletMinCoord2, outletMaxCoord2) };

      auto sphBoundaryConditions = std::make_shared<SPHBoundaryConditions>(inletCoords, outletCoords, inletNormal,
        outletNormals, inletRadius, inletCenterPoint,  inletFlowRate, particles, wallParticles);
      sphModel->setBoundaryConditions(sphBoundaryConditions);

      speedOfSound = 300;
  }
  else if (SCENE_ID == 4)
  {
      // pipe flow with leak
      auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(MeshIO::read(iMSTK_DATA_ROOT "/cylinder/cylinder_small.stl"));
      auto surfMeshShell = std::dynamic_pointer_cast<SurfaceMesh>(MeshIO::read(iMSTK_DATA_ROOT "/cylinder/cylinder_small_shell_cut_ellipse.stl"));
      auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(MeshIO::read(iMSTK_DATA_ROOT "/cylinder/cylinder_small.vtk"));

      //surfMesh->rotate(Vec3d(1, 0, 0), PI / 5, Geometry::TransformType::ConcatenateToTransform);
      //surfMeshShell->rotate(Vec3d(1, 0, 0), PI / 5, Geometry::TransformType::ConcatenateToTransform);
      //tetMesh->rotate(Vec3d(1, 0, 0), PI / 5, Geometry::TransformType::ConcatenateToTransform);

      // set tetrahedral mesh used when writing VTUs
      sphModel->setGeometryMesh(tetMesh);

      // fill grid with points
      Vec3d aabbMin, aabbMax;
      surfMeshShell->computeBoundingBox(aabbMin, aabbMax, 1.);
      const double length = std::abs(aabbMax.x() - aabbMin.x());
      const double width = std::abs(aabbMax.y() - aabbMin.y());
      const double depth = std::abs(aabbMax.z() - aabbMin.z());
      const auto spacing = 2.2 * particleRadius;
      const auto wallSpacing = 1.9 * particleRadius;
      const auto nx = static_cast<size_t>(length / spacing);
      const auto ny = static_cast<size_t>(width / spacing);
      const auto nz = static_cast<size_t>(depth / spacing);
      const auto nx_wall = static_cast<size_t>(length / wallSpacing);
      const auto ny_wall = static_cast<size_t>(width / wallSpacing);
      const auto nz_wall = static_cast<size_t>(depth / wallSpacing);
      auto uniformMesh = std::dynamic_pointer_cast<PointSet>(GeometryUtils::createUniformMesh(aabbMin, aabbMax, nx, ny, nz));
      auto uniformMesh_wall = std::dynamic_pointer_cast<PointSet>(GeometryUtils::createUniformMesh(aabbMin, aabbMax, nx_wall, ny_wall, nz_wall));

      auto enclosedFluidPoints = GeometryUtils::getEnclosedPoints(surfMesh, uniformMesh, false);
      particles = enclosedFluidPoints->getInitialVertexPositions();
      auto enclosedWallPoints = GeometryUtils::getEnclosedPoints(surfMeshShell, uniformMesh_wall, false);
      StdVectorOfVec3d wallParticles = enclosedWallPoints->getInitialVertexPositions();

      // set up inlet boundary conditions
      double inletFlowRate = 30;
      double inletRadius = 0.6;
      Vec3d inletCenterPoint = Vec3d(-2.5, 0.0, 0);
      Vec3d inletMinCoord = inletCenterPoint - Vec3d(0.1, inletRadius, inletRadius);
      Vec3d inletMaxCoord = inletCenterPoint + Vec3d(0.5, inletRadius, inletRadius);
      Vec3d inletNormal(-1, 0, 0);
      std::pair<Vec3d, Vec3d> inletCoords = std::make_pair(inletMinCoord, inletMaxCoord);

      // set up outlet boundary conditions
      double outletRadius = 0.5;
      Vec3d outletCenterPoint = Vec3d(2.5, 0.0, 0.0);

      Vec3d outletMinCoord = outletCenterPoint - Vec3d(0.5, outletRadius, outletRadius);
      Vec3d outletMaxCoord = outletCenterPoint + Vec3d(0, outletRadius, outletRadius);
      StdVectorOfVec3d outletNormals{ Vec3d(1, 0, 0) };
      std::vector<std::pair<Vec3d, Vec3d>> outletCoords{ std::make_pair(outletMinCoord, outletMaxCoord) };

      auto sphBoundaryConditions = std::make_shared<SPHBoundaryConditions>(inletCoords, outletCoords, inletNormal, outletNormals, inletRadius, inletCenterPoint, inletFlowRate, particles, wallParticles);
      sphModel->setBoundaryConditions(sphBoundaryConditions);
      const Vec3d hemorrhagePlaneCenter(0, 0.42, -0.48);
      const double hemorrhagePlaneRadius = 0.4;
      const double hemorrhagePlaneArea = 0.16;
      const Vec3d hemorrhagePlaneOutwardNormal(0, 1, -1);
      auto sphHemorrhageModel = std::make_shared<SPHHemorrhage>(hemorrhagePlaneCenter, hemorrhagePlaneRadius, hemorrhagePlaneArea, hemorrhagePlaneOutwardNormal);
      sphModel->setHemorrhageModel(sphHemorrhageModel);
  }

  else if (SCENE_ID == 5)
  {
      // femoral artery flow with leak
      auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(MeshIO::read(iMSTK_DATA_ROOT "/femoral/femoral_artery.stl"));
      auto surfMeshShell = std::dynamic_pointer_cast<SurfaceMesh>(MeshIO::read(iMSTK_DATA_ROOT "/femoral/femoral_artery_shell_cut_ellipse.stl"));
      auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(MeshIO::read(iMSTK_DATA_ROOT "/femoral/femoral_artery.vtk"));

      //surfMesh->rotate(Vec3d(1, 0, 0), PI / 5, Geometry::TransformType::ConcatenateToTransform);
      //surfMeshShell->rotate(Vec3d(1, 0, 0), PI / 5, Geometry::TransformType::ConcatenateToTransform);
      //tetMesh->rotate(Vec3d(1, 0, 0), PI / 5, Geometry::TransformType::ConcatenateToTransform);

      // set tetrahedral mesh used when writing VTUs
      sphModel->setGeometryMesh(tetMesh);

      // fill grid with points
      Vec3d aabbMin, aabbMax;
      surfMeshShell->computeBoundingBox(aabbMin, aabbMax, 1.);
      const double length = std::abs(aabbMax.x() - aabbMin.x());
      const double width = std::abs(aabbMax.y() - aabbMin.y());
      const double depth = std::abs(aabbMax.z() - aabbMin.z());
      const auto spacing = 2.2 * particleRadius;
      const auto wallSpacing = 1.9 * particleRadius;
      const auto nx = static_cast<size_t>(length / spacing);
      const auto ny = static_cast<size_t>(width / spacing);
      const auto nz = static_cast<size_t>(depth / spacing);
      const auto nx_wall = static_cast<size_t>(length / wallSpacing);
      const auto ny_wall = static_cast<size_t>(width / wallSpacing);
      const auto nz_wall = static_cast<size_t>(depth / wallSpacing);
      auto uniformMesh = std::dynamic_pointer_cast<PointSet>(GeometryUtils::createUniformMesh(aabbMin, aabbMax, nx, ny, nz));
      auto uniformMesh_wall = std::dynamic_pointer_cast<PointSet>(GeometryUtils::createUniformMesh(aabbMin, aabbMax, nx_wall, ny_wall, nz_wall));

      auto enclosedFluidPoints = GeometryUtils::getEnclosedPoints(surfMesh, uniformMesh, false);
      particles = enclosedFluidPoints->getInitialVertexPositions();
      auto enclosedWallPoints = GeometryUtils::getEnclosedPoints(surfMeshShell, uniformMesh_wall, false);
      StdVectorOfVec3d wallParticles = enclosedWallPoints->getInitialVertexPositions();

      // set up inlet boundary conditions
      double inletFlowRate = 3.0;
      double inletRadius = 0.12;
      Vec3d inletCenterPoint = Vec3d(-2.07, 0.36, -0.52);
      Vec3d inletMinCoord = inletCenterPoint - Vec3d(0.01, inletRadius, inletRadius);
      Vec3d inletMaxCoord = inletCenterPoint + Vec3d(0.2, inletRadius, inletRadius);
      Vec3d inletNormal(-1, 0, 0);
      std::pair<Vec3d, Vec3d> inletCoords = std::make_pair(inletMinCoord, inletMaxCoord);

      // set up outlet boundary conditions
      double outletRadius = 0.15;
      Vec3d outletCenterPoint = Vec3d(2.66, -0.21, 0.72);

      Vec3d outletMinCoord = outletCenterPoint - Vec3d(0.3, outletRadius, outletRadius);
      Vec3d outletMaxCoord = outletCenterPoint + Vec3d(0, outletRadius, outletRadius);
      StdVectorOfVec3d outletNormals{ Vec3d(1, 0, 0) };
      std::vector<std::pair<Vec3d, Vec3d>> outletCoords{ std::make_pair(outletMinCoord, outletMaxCoord) };

      auto sphBoundaryConditions = std::make_shared<SPHBoundaryConditions>(inletCoords, outletCoords, inletNormal, outletNormals, inletRadius, inletCenterPoint, inletFlowRate, particles, wallParticles);
      sphModel->setBoundaryConditions(sphBoundaryConditions);
      const Vec3d hemorrhagePlaneCenter(0.57, -0.16, -0.12);
      const double hemorrhagePlaneRadius = 0.4;
      const double hemorrhagePlaneArea = 0.018;
      const Vec3d hemorrhagePlaneNormal(0.31, -0.14, -0.94);
      auto sphHemorrhageModel = std::make_shared<SPHHemorrhage>(hemorrhagePlaneCenter, hemorrhagePlaneRadius, hemorrhagePlaneArea, hemorrhagePlaneNormal);
      sphModel->setHemorrhageModel(sphHemorrhageModel);

      sphModel->setInitialVelocities(particles.size(), Vec3d(80, 0, 0));

      speedOfSound = 800;
  }
  
  sphModel->setWriteToOutputModulo(0.1);

  LOG(INFO) << "Number of particles: " << particles.size();

  // Create a geometry object
  auto fluidGeometry = std::make_shared<PointSet>();
  fluidGeometry->initialize(particles);

  // Create a visual model
  auto fluidVisualModel = std::make_shared<VisualModel>(fluidGeometry);
  auto fluidMaterial = std::make_shared<RenderMaterial>();
  fluidMaterial->setDisplayMode(RenderMaterial::DisplayMode::Points);
  fluidMaterial->setVertexColor(Color(1, 0, 1, 0.2));
  fluidVisualModel->setRenderMaterial(fluidMaterial);

  sphModel->setModelGeometry(fluidGeometry);

  // configure model
  auto sphParams = std::make_shared<SPHModelConfig>(particleRadius, speedOfSound, restDensity);
  sphParams->m_gravity = Vec3d(0, 0, 0);
  
  fluidMaterial->setPointSize(5.0);
  sphParams->m_dynamicViscosityCoeff = 1.0;

  if (SCENE_ID == 5)
  {
    fluidMaterial->setPointSize(1.8);
  }

  sphParams->m_bNormalizeDensity = true;

  sphModel->configure(sphParams);
  sphModel->setTimeStepSizeType(TimeSteppingType::RealTime);

  // Add the component models
  auto fluidObj = std::make_shared<SPHObject>("Sphere");
  fluidObj->addVisualModel(fluidVisualModel);
  fluidObj->setCollidingGeometry(fluidGeometry);
  fluidObj->setDynamicalModel(sphModel);
  fluidObj->setPhysicsGeometry(fluidGeometry);
  scene->addSceneObject(fluidObj);
   
  return fluidObj;
}
