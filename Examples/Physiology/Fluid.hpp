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
/// \brief Generate fluid for pipe flow
///
StdVectorOfVec3d
generatePipeFluid(const double particleRadius)
{
  const double pipeRadius = 1.0;
  const double pipeLength = 5.0;
  const Vec3d  lcorner(-5, 4, -2);
  const Vec3d pipeLeftCenter = lcorner + Vec3d(0, pipeRadius, pipeRadius);

  const auto spacing = 2.0 * particleRadius;
  const auto N_width = static_cast<size_t>(2.0 * pipeRadius / spacing); // Maximum number of particles in width dimension
  const auto N_length = static_cast<size_t>(pipeLength / spacing); // Maximum number of particles in length dimension

  StdVectorOfVec3d particles;
  particles.reserve(N_width * N_width * N_length);

  for (size_t i = 0; i < N_length; ++i)
  {
    for (size_t j = 0; j < N_width; ++j)
    {
      for (size_t k = 0; k < N_width; ++k)
      {
        Vec3d ppos = lcorner + Vec3d(spacing * double(i), spacing * double(j), spacing * double(k));
        //const double cx = ppos.x() - pipeBottomCenter.x();
        //const double cy = ppos.y() - pipeBottomCenter.y();
        Vec3d cx = ppos - Vec3d(spacing * double(i), 0, 0) - pipeLeftCenter;
        if (cx.squaredNorm() < pipeRadius)
        {
          particles.push_back(ppos);
        }
      }
    }
  }

   return particles;
}

///
/// \brief Get the center of a mesh
///
Vec3d getCenter(std::shared_ptr<imstk::PointSet>& meshPoints)
{
  auto vtkPoints = GeometryUtils::convertPointSetToVtkPointSet(meshPoints);

  // Compute the center of mass
  vtkSmartPointer<vtkCenterOfMass> centerOfMassFilter =
    vtkSmartPointer<vtkCenterOfMass>::New();
  centerOfMassFilter->SetInputData(vtkPoints);
  centerOfMassFilter->SetUseScalarsAsWeights(false);
  centerOfMassFilter->Update();

  double center[3];
  centerOfMassFilter->GetCenter(center);

  return Vec3d(center);
}

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

///
/// \brief Initialize fluid particle velocities
///
StdVectorOfVec3d
initializeNonZeroVelocities(const size_t numParticles)
{
  StdVectorOfVec3d initialVelocities(numParticles, Vec3d(0.0, 0, 0));
  return initialVelocities;
}

std::shared_ptr<SPHObject>
generateFluid(const std::shared_ptr<Scene>& scene, const double particleRadius)
{
  StdVectorOfVec3d particles;
  auto sphModel = std::make_shared<SPHModel>();

  if (SCENE_ID == 1)
  {
    // pipe flow
    auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(MeshIO::read(iMSTK_DATA_ROOT "/cylinder/cylinder.stl"));
    std::shared_ptr<SurfaceMesh> surfMeshSmall = std::make_shared<SurfaceMesh>(*surfMesh);
    auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(MeshIO::read(iMSTK_DATA_ROOT "/cylinder/cylinder.vtk"));
    std::shared_ptr<SurfaceMesh> surfMeshExpanded = std::make_shared<SurfaceMesh>(*surfMesh);

    // set tetrahedral mesh used when writing VTUs
    sphModel->setGeometryMesh(tetMesh);

    // scale meshes to create walls
    const double scale = 1.5;
    surfMeshExpanded->scale(scale, Geometry::TransformType::ApplyToData);
    surfMesh->directionalScale(scale + 0.1, 1.0, 1.0);
    // translate expanded mesh on top of original mesh so that we can subtract them and get a wall mesh
    Vec3d originalCylCenter = getCenter(std::dynamic_pointer_cast<PointSet>(surfMeshSmall));
    Vec3d directionalExpandedCylCenter = getCenter(std::dynamic_pointer_cast<PointSet>(surfMesh));
    Vec3d expandedCylCenter = getCenter(std::dynamic_pointer_cast<PointSet>(surfMeshExpanded));
    surfMeshExpanded->translate(originalCylCenter - expandedCylCenter, Geometry::TransformType::ApplyToData);
    surfMesh->translate(originalCylCenter - directionalExpandedCylCenter, Geometry::TransformType::ApplyToData);

    std::shared_ptr<SurfaceMesh> wallMesh = generateWallFluidPoints(particleRadius, surfMesh, surfMeshExpanded);

    // find inlet and outlet domains
    Vec3d aabbMin1, aabbMax1;
    surfMeshSmall->computeBoundingBox(aabbMin1, aabbMax1, 1.);
    Vec3d inletMinCoord = aabbMin1;
    Vec3d inletMaxCoord = Vec3d(aabbMin1.x() + 1, aabbMax1.y(), aabbMax1.z());
    Vec3d outletMinCoord = Vec3d(aabbMax1.x() - 1, aabbMin1.y(), aabbMin1.z());
    Vec3d outletMaxCoord = aabbMax1;

    // fill grid with points
    Vec3d aabbMin, aabbMax;
    surfMeshExpanded->computeBoundingBox(aabbMin, aabbMax, 1.);
    const double length = std::abs(aabbMax.x() - aabbMin.x());
    const double width = std::abs(aabbMax.y() - aabbMin.y());
    const double depth = std::abs(aabbMax.z() - aabbMin.z());
    const auto spacing = 2.0 * particleRadius;
    const auto nx = static_cast<size_t>(length / spacing);
    const auto ny = static_cast<size_t>(width / spacing);
    const auto nz = static_cast<size_t>(depth / spacing);

    auto uniformMesh = std::dynamic_pointer_cast<PointSet>(GeometryUtils::createUniformMesh(aabbMin, aabbMax, nx, ny, nz));
    auto enclosedFluidPoints = GeometryUtils::getEnclosedPoints(surfMeshSmall, uniformMesh, false);
    particles = enclosedFluidPoints->getInitialVertexPositions();
    auto enclosedWallPoints = GeometryUtils::getEnclosedPoints(wallMesh, uniformMesh, false);
    StdVectorOfVec3d wallParticles = enclosedWallPoints->getInitialVertexPositions();

    // set up boundary conditions
    //Vec3d inletVelocity(1, 0.0, 0.0);
    double inletFlowRate = 10;
    Vec3d inletNormal(1, 0, 0);
    auto sphBoundaryConditions = std::make_shared<SPHBoundaryConditions>(std::make_pair(inletMinCoord, inletMaxCoord), inletNormal,
      std::make_pair(outletMinCoord, outletMaxCoord), inletFlowRate, particles, wallParticles);
    sphModel->setBoundaryConditions(sphBoundaryConditions);

    StdVectorOfVec3d initialFluidVelocities = initializeNonZeroVelocities(particles.size());
    sphModel->setInitialVelocities(initialFluidVelocities);
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
    const auto spacing = 2.0 * particleRadius;
    const auto nx = static_cast<size_t>(length / spacing);
    const auto ny = static_cast<size_t>(width / spacing);
    const auto nz = static_cast<size_t>(depth / spacing);
    auto uniformMesh = std::dynamic_pointer_cast<PointSet>(GeometryUtils::createUniformMesh(aabbMin, aabbMax, nx, ny, nz));
    auto enclosedFluidPoints = GeometryUtils::getEnclosedPoints(surfMesh, uniformMesh, false);
    particles = enclosedFluidPoints->getInitialVertexPositions();
    auto enclosedWallPoints = GeometryUtils::getEnclosedPoints(surfMeshShell, uniformMesh, false);
    StdVectorOfVec3d wallParticles = enclosedWallPoints->getInitialVertexPositions();

    // set up boundary conditions
    //Vec3d inletVelocity = Vec3d(1, 0.0, 0.0);
    double inletFlowRate = 10;
    Vec3d inletCenterPoint = Vec3d(-2.2, 2.0, 0.0);
    Vec3d outletCenterPoint = Vec3d(-2.2, -2.0, 0.0);
    double inletRadius = 0.6;
    double outletRadius = 0.6;
    Vec3d inletMinCoord = inletCenterPoint - Vec3d(0, inletRadius, inletRadius);
    Vec3d inletMaxCoord = inletCenterPoint + Vec3d(1.0, inletRadius, inletRadius);
    Vec3d outletMinCoord = outletCenterPoint - Vec3d(0, outletRadius, outletRadius);
    Vec3d outletMaxCoord = outletCenterPoint + Vec3d(1.0, outletRadius, outletRadius);
    Vec3d inletNormal(1, 0, 0);
    auto sphBoundaryConditions = std::make_shared<SPHBoundaryConditions>(std::make_pair(inletMinCoord, inletMaxCoord), inletNormal,
      std::make_pair(outletMinCoord, outletMaxCoord), inletFlowRate, particles, wallParticles);
    sphModel->setBoundaryConditions(sphBoundaryConditions);

    StdVectorOfVec3d initialFluidVelocities = initializeNonZeroVelocities(particles.size());
    sphModel->setInitialVelocities(initialFluidVelocities);
  }

  else if (SCENE_ID == 3)
  {
  // bifurcation flow
  auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(MeshIO::read(iMSTK_DATA_ROOT "/bifurcation/bifurcation_small.stl"));
  auto surfMeshShell = std::dynamic_pointer_cast<SurfaceMesh>(MeshIO::read(iMSTK_DATA_ROOT "/bifurcation/bifurcation_small_shell.stl"));
  auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(MeshIO::read(iMSTK_DATA_ROOT "/bifurcation/bifurcation_small.vtk"));

  surfMesh->scale(0.7, Geometry::TransformType::ApplyToData);
  surfMeshShell->scale(0.7, Geometry::TransformType::ApplyToData);
  tetMesh->scale(0.7, Geometry::TransformType::ApplyToData);

  // set tetrahedral mesh used when writing VTUs
  sphModel->setGeometryMesh(tetMesh);

  // fill grid with points
  Vec3d aabbMin, aabbMax;
  surfMeshShell->computeBoundingBox(aabbMin, aabbMax, 1.);
  const double length = std::abs(aabbMax.x() - aabbMin.x());
  const double width = std::abs(aabbMax.y() - aabbMin.y());
  const double depth = std::abs(aabbMax.z() - aabbMin.z());
  const auto spacing = 2.0 * particleRadius;
  const auto nx = static_cast<size_t>(length / spacing);
  const auto ny = static_cast<size_t>(width / spacing);
  const auto nz = static_cast<size_t>(depth / spacing);
  auto uniformMesh = std::dynamic_pointer_cast<PointSet>(GeometryUtils::createUniformMesh(aabbMin, aabbMax, nx, ny, nz));
  auto enclosedFluidPoints = GeometryUtils::getEnclosedPoints(surfMesh, uniformMesh, false);
  particles = enclosedFluidPoints->getInitialVertexPositions();
  auto enclosedWallPoints = GeometryUtils::getEnclosedPoints(surfMeshShell, uniformMesh, false);
  StdVectorOfVec3d wallParticles = enclosedWallPoints->getInitialVertexPositions();

  // set up boundary conditions
  //Vec3d inletVelocity = Vec3d(1, 0.0, 0.0);
  double inletFlowRate = 10;
  Vec3d inletCenterPoint = Vec3d(-5.81, 0.0, 0.0);
  double inletRadius = 1.05;
  Vec3d inletMinCoord = inletCenterPoint - Vec3d(0, inletRadius, inletRadius);
  Vec3d inletMaxCoord = inletCenterPoint + Vec3d(1.0, inletRadius, inletRadius);
  Vec3d outletMinCoord = Vec3d(5.6, -1.295, -7.21);
  Vec3d outletMaxCoord = Vec3d(6.3, 1.352, 7.21);
  Vec3d inletNormal(1, 0, 0);
  auto sphBoundaryConditions = std::make_shared<SPHBoundaryConditions>(std::make_pair(inletMinCoord, inletMaxCoord), inletNormal,
    std::make_pair(outletMinCoord, outletMaxCoord), inletFlowRate, particles, wallParticles);
  sphModel->setBoundaryConditions(sphBoundaryConditions);

  StdVectorOfVec3d initialFluidVelocities = initializeNonZeroVelocities(particles.size());
  sphModel->setInitialVelocities(initialFluidVelocities);
  }

  else if (SCENE_ID == 4)
  {
  // pipe flow with leak
  auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(MeshIO::read(iMSTK_DATA_ROOT "/cylinder/cylinder.stl"));
  auto surfMeshShell = std::dynamic_pointer_cast<SurfaceMesh>(MeshIO::read(iMSTK_DATA_ROOT "/cylinder/cylinder_hole.stl"));
  auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(MeshIO::read(iMSTK_DATA_ROOT "/cylinder/cylinder.vtk"));

  // set tetrahedral mesh used when writing VTUs
  sphModel->setGeometryMesh(tetMesh);
  
  // fill grid with points
  Vec3d aabbMin, aabbMax;
  surfMeshShell->computeBoundingBox(aabbMin, aabbMax, 1.);
  const double length = std::abs(aabbMax.x() - aabbMin.x());
  const double width = std::abs(aabbMax.y() - aabbMin.y());
  const double depth = std::abs(aabbMax.z() - aabbMin.z());
  const auto spacing = 2.0 * particleRadius;
  const auto nx = static_cast<size_t>(length / spacing);
  const auto ny = static_cast<size_t>(width / spacing);
  const auto nz = static_cast<size_t>(depth / spacing);
  auto uniformMesh = std::dynamic_pointer_cast<PointSet>(GeometryUtils::createUniformMesh(aabbMin, aabbMax, nx, ny, nz));
  auto enclosedFluidPoints = GeometryUtils::getEnclosedPoints(surfMesh, uniformMesh, false);
  particles = enclosedFluidPoints->getInitialVertexPositions();
  auto enclosedWallPoints = GeometryUtils::getEnclosedPoints(surfMeshShell, uniformMesh, false);
  StdVectorOfVec3d wallParticles = enclosedWallPoints->getInitialVertexPositions();

  // set up boundary conditions
  //Vec3d inletVelocity = Vec3d(1, 0.0, 0.0);
  double inletFlowRate = 10;
  Vec3d inletCenterPoint = Vec3d(-8.5, 6.0, 2.0);
  Vec3d outletCenterPoint = Vec3d(0.5, 6.0, 2.0);
  double inletRadius = 1.6;
  double outletRadius = 1.6;
  Vec3d inletMinCoord = inletCenterPoint - Vec3d(0, inletRadius, inletRadius);
  Vec3d inletMaxCoord = inletCenterPoint + Vec3d(1.0, inletRadius, inletRadius);
  Vec3d outletMinCoord = outletCenterPoint - Vec3d(1.0, outletRadius, outletRadius);
  Vec3d outletMaxCoord = outletCenterPoint + Vec3d(0, outletRadius, outletRadius);
  Vec3d inletNormal(1, 0, 0);
  auto sphBoundaryConditions = std::make_shared<SPHBoundaryConditions>(std::make_pair(inletMinCoord, inletMaxCoord), inletNormal,
    std::make_pair(outletMinCoord, outletMaxCoord), inletFlowRate, particles, wallParticles);
  sphModel->setBoundaryConditions(sphBoundaryConditions);

  Vec3d hemorrhagePlaneCenter(-4.16, 4.03, 1.97);
  double hemorrhagePlaneRadius = 0.5;
  Vec3d hemorrhagePlaneNormal(0, 1, 0);
  auto sphHemorrhageModel = std::make_shared<SPHHemorrhage>(hemorrhagePlaneCenter, hemorrhagePlaneRadius, hemorrhagePlaneNormal);
  sphModel->setHemorrhageModel(sphHemorrhageModel);

  StdVectorOfVec3d initialFluidVelocities = initializeNonZeroVelocities(particles.size());
  sphModel->setInitialVelocities(initialFluidVelocities);
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
  fluidMaterial->setPointSize(4.);

  fluidVisualModel->setRenderMaterial(fluidMaterial);

  sphModel->setModelGeometry(fluidGeometry);

  // configure model
  auto sphParams = std::make_shared<SPHModelConfig>(particleRadius);
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
