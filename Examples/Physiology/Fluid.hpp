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


using namespace imstk;

///
/// \brief Generate parallel plate flow
///
StdVectorOfVec3d
generateParallelPlatesFlow(const double particleRadius, std::shared_ptr<SPHModel>& sphModel)
{
  /* 
  Set the fluid channel dimensions. For now, we don't need the fluid channel to be as long as the
  parallel plates
  */
  const double channelLength = 5.0;
  const double channelWidth = 3.0;
  const double channelDepth = 5.0;

  const Vec3d channelLowerCorner(-8, 2, 0);

  const auto spacing = 2.0 * particleRadius;
  const auto nx = static_cast<size_t>(channelLength / spacing);
  const auto ny = static_cast<size_t>(channelWidth / spacing);
  const auto nz = static_cast<size_t>(channelDepth / spacing);

  StdVectorOfVec3d particles;

  for (size_t i = 0; i < nx; ++i)
  {
    for (size_t j = 0; j < ny; ++j)
    {
      for (size_t k = 0; k < nz; ++k)
      {
        Vec3d ppos = channelLowerCorner + Vec3d(spacing * double(i), spacing * double(j), spacing * double(k));
        particles.push_back(ppos);
      }
    }
  }

  // generate fixed particles for the top plate no slip boundary condition
  StdVectorOfVec3d plateParticles;

  const double plateLength = 10.0;
  const double plateWidth = 1.0;
  const double plateDepth = 5.0;

  const auto nxPlate = static_cast<size_t>(plateLength / spacing);
  const auto nyPlate = static_cast<size_t>(plateWidth / spacing);
  const auto nzPlate = static_cast<size_t>(plateDepth / spacing);

  const Vec3d topPlateLowerCorner(channelLowerCorner.x(), channelLowerCorner.y() + channelWidth, channelLowerCorner.z());
  for (size_t i = 0; i < nxPlate; ++i)
  {
    for (size_t j = 0; j < nyPlate; ++j)
    {
      for (size_t k = 0; k < nzPlate; ++k)
      {
        Vec3d ppos = topPlateLowerCorner + Vec3d(spacing * double(i), spacing * double(j), spacing * double(k));
        plateParticles.push_back(ppos);
      }
    }
  }

  // generate fixed particles for the bottom plate no slip boundary condition
  const Vec3d bottomPlateLowerCorner(channelLowerCorner.x(), channelLowerCorner.y() - plateWidth, channelLowerCorner.z());
  for (size_t i = 0; i < nxPlate; ++i)
  {
    for (size_t j = 0; j < nyPlate; ++j)
    {
      for (size_t k = 0; k < nzPlate; ++k)
      {
        Vec3d ppos = bottomPlateLowerCorner + Vec3d(spacing * double(i), spacing * double(j), spacing * double(k));
        plateParticles.push_back(ppos);
      }
    }
  }

  // get wallParticles positions within combined vector
  std::vector<size_t> wallParticlesIndices(plateParticles.size());
  std::iota(wallParticlesIndices.begin(), wallParticlesIndices.end(), particles.size());
  sphModel->setWallPointIndices(wallParticlesIndices);

  // combine the wall particles with the fluid particles
  particles.insert(particles.end(), plateParticles.begin(), plateParticles.end());

  // set the coordinates needed for periodic boundary conditions
  sphModel->setMinMaxXCoords(channelLowerCorner.x(), channelLowerCorner.x() + plateLength);

  return particles;
}

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
  // translate expanded mesh on top of original mesh so that we can subtract them and get a wall mesh
  Vec3d originalCylCenter = getCenter(std::dynamic_pointer_cast<PointSet>(surfMesh));
  Vec3d expandedCylCenter = getCenter(std::dynamic_pointer_cast<PointSet>(surfMeshExpanded));
  surfMeshExpanded->translate(originalCylCenter - expandedCylCenter, Geometry::TransformType::ApplyToData);

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
  StdVectorOfVec3d initialVelocities(numParticles, Vec3d(0, 0, 0));
  return initialVelocities;
}

std::shared_ptr<SPHObject>
generateFluid(const std::shared_ptr<Scene>& scene, const double particleRadius)
{
  StdVectorOfVec3d particles;
  auto sphModel = std::make_shared<SPHModel>();

  if (SCENE_ID == 1)
  {
    auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(MeshIO::read(iMSTK_DATA_ROOT "/cylinder/cylinder.stl"));
    std::shared_ptr<SurfaceMesh> surfMeshSmall = std::make_shared<SurfaceMesh>(*surfMesh);
    auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(MeshIO::read(iMSTK_DATA_ROOT "/cylinder/cylinder.vtk"));

    sphModel->setGeometryMesh(tetMesh);

    std::shared_ptr<SurfaceMesh> surfMeshExpanded = std::make_shared<SurfaceMesh>(*surfMesh);

    const double scale = 1.5;
    surfMeshExpanded->scale(scale, Geometry::TransformType::ApplyToData);

    surfMesh->directionalScale(scale + 0.1, 1.0, 1.0);
    std::shared_ptr<SurfaceMesh> wallMesh = generateWallFluidPoints(particleRadius, surfMesh, surfMeshExpanded);

    Vec3d aabbMin1, aabbMax1;
    surfMeshSmall->computeBoundingBox(aabbMin1, aabbMax1, 1.);
    sphModel->setInletRegionXCoord(aabbMin1.x() + 1);
    sphModel->setOutletRegionXCoord(aabbMax1.x() - 1);

    sphModel->setMinMaxXCoords(aabbMin1.x(), aabbMax1.x());

    // compute center and radius of inlet
    Vec3d inletCenterPoint = Vec3d(aabbMin1.x(), (aabbMax1.y() + aabbMin1.y()) / 2.0, (aabbMax1.z() + aabbMin1.z()) / 2.0);
    double inletRadius = (aabbMax1.y() - aabbMin1.y()) / 2.0;
    sphModel->setInletRadius(inletRadius);
    sphModel->setInletCenterPoint(inletCenterPoint);

    Vec3d aabbMin, aabbMax;
    surfMeshExpanded->computeBoundingBox(aabbMin, aabbMax, 1.);

    const double length = aabbMax.x() - aabbMin.x();
    const double width = aabbMax.y() - aabbMin.y();
    const double depth = aabbMax.z() - aabbMin.z();

    const auto spacing = 2.0 * particleRadius;
    const auto nx = static_cast<size_t>(length / spacing);
    const auto ny = static_cast<size_t>(width / spacing);
    const auto nz = static_cast<size_t>(depth / spacing);

    auto uniformMesh = std::dynamic_pointer_cast<PointSet>(GeometryUtils::createUniformMesh(aabbMin, aabbMax, nx, ny, nz));
    auto enclosedFluidPoints = GeometryUtils::getEnclosedPoints(surfMeshSmall, uniformMesh, false);
    particles = enclosedFluidPoints->getInitialVertexPositions();
    auto enclosedWallPoints = GeometryUtils::getEnclosedPoints(wallMesh, uniformMesh, false);
    StdVectorOfVec3d wallParticles = enclosedWallPoints->getInitialVertexPositions();

    // get wallParticles positions within combined vector
    std::vector<size_t> wallParticlesIndices(wallParticles.size());
    std::iota(wallParticlesIndices.begin(), wallParticlesIndices.end(), particles.size());

    particles.insert(particles.end(), wallParticles.begin(), wallParticles.end());

    // add wall particles
    sphModel->setWallPointIndices(wallParticlesIndices);

    // buffer domain
    StdVectorOfVec3d bufferParticles;
    double bufferXCoordMin = 1;
    sphModel->setBufferXCoord(bufferXCoordMin);
    for (int i = 0; i < 2000; i++)
    {
      bufferParticles.push_back(Vec3d(bufferXCoordMin, 0, 0));
    }
    std::vector<size_t> bufferParticlesIndices(bufferParticles.size());
    std::iota(bufferParticlesIndices.begin(), bufferParticlesIndices.end(), particles.size());
    sphModel->setBufferParticleIndices(bufferParticlesIndices);

    particles.insert(particles.end(), bufferParticles.begin(), bufferParticles.end());

    StdVectorOfVec3d initialFluidVelocities = initializeNonZeroVelocities(particles.size());

    sphModel->setInitialVelocities(initialFluidVelocities);
  }

  else if (SCENE_ID == 2)
  {
    particles = generateParallelPlatesFlow(particleRadius, sphModel);

    StdVectorOfVec3d initialFluidVelocities = initializeNonZeroVelocities(particles.size());

    sphModel->setInitialVelocities(initialFluidVelocities);
  }
  
  sphModel->setWriteToOutputModulo(0.5);
  //sphModel->setInletDensity(1001);
  sphModel->setInletVelocity(Vec3d(0.3, 0.0, 0.0));
  sphModel->setOutletDensity(1000);

  LOG(INFO) << "Number of particles: " << particles.size();

  // Create a geometry object
  auto fluidGeometry = std::make_shared<PointSet>();
  fluidGeometry->initialize(particles);
  auto fluidVisualModel = std::make_shared<VisualModel>(fluidGeometry);
  auto fluidMaterial = std::make_shared<RenderMaterial>();
  fluidMaterial->setColor(Color(1, 0, 1, 0));
  fluidMaterial->setSphereGlyphSize(particleRadius);
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
