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

#include "imstkMath.h"
#include "imstkTimer.h"
#include "imstkSimulationManager.h"

// Objects
#include "imstkForceModelConfig.h"
#include "imstkFEMDeformableBodyModel.h"
#include "imstkVirtualCouplingPBDObject.h"
#include "imstkDynamicObject.h"
#include "imstkDeformableObject.h"
#include "imstkPbdObject.h"
#include "imstkSceneObject.h"
#include "imstkLight.h"
#include "imstkCamera.h"
#include "imstkRigidObject.h"


#include "imstkGraph.h"

// Time Integrators
#include "imstkBackwardEuler.h"

// Solvers
#include "imstkNonlinearSystem.h"
#include "imstkNewtonSolver.h"
#include "imstkConjugateGradient.h"
#include "imstkPbdSolver.h"
#include "imstkGaussSeidel.h"
#include "imstkJacobi.h"
#include "imstkSOR.h"

// Geometry
#include "imstkPlane.h"
#include "imstkSphere.h"
#include "imstkCube.h"
#include "imstkCylinder.h"
#include "imstkTetrahedralMesh.h"
#include "imstkHexahedralMesh.h"
#include "imstkSurfaceMesh.h"
#include "imstkMeshIO.h"
#include "imstkLineMesh.h"
#include "imstkDecalPool.h"

// Maps
#include "imstkTetraTriangleMap.h"
#include "imstkIsometricMap.h"
#include "imstkOneToOneMap.h"

// Devices and controllers
#include "imstkHDAPIDeviceClient.h"
#include "imstkHDAPIDeviceServer.h"
#include "imstkVRPNDeviceClient.h"
#include "imstkVRPNDeviceServer.h"
#include "imstkCameraController.h"
#include "imstkSceneObjectController.h"
#include "imstkLaparoscopicToolController.h"

// Collisions
#include "imstkInteractionPair.h"
#include "imstkPointSetToPlaneCD.h"
#include "imstkPointSetToSphereCD.h"
#include "imstkVirtualCouplingCH.h"
#include "imstkPointSetToSpherePickingCD.h"
#include "imstkPickingCH.h"
#include "imstkBoneDrillingCH.h"

// logger
#include "g3log/g3log.hpp"
#include "imstkLogger.h"

// imstk utilities
#include "imstkPlotterUtils.h"
#include "imstkAPIUtilities.h"

// testVTKTexture
#include <vtkOBJReader.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <string>
#include <vtkJPEGReader.h>

#ifdef iMSTK_AUDIO_ENABLED

// Audio
#include <SFML/Audio.hpp>

#endif

// global variables
const std::string phantomOmni1Name = "Phantom1";
const std::string phantomOmni2Name = "Phantom2";
const std::string novintFalcon1Name = "device0";
const std::string novintFalcon2Name = "device1";

using namespace imstk;

void testPbdFluid()
{
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("PBDFluidTest");

    scene->getCamera()->setPosition(0, 10.0, 15.0);

    // dragon
    auto tetMesh = MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");
    if (!tetMesh)
    {
        LOG(WARNING) << "Could not read mesh from file.";
        return;
    }

    auto fluidMesh = std::make_shared<PointSet>();
    fluidMesh->initialize(tetMesh->getInitialVertexPositions());

    auto material1 = std::make_shared<RenderMaterial>();
    material1->setDiffuseColor(Color::Blue);
    material1->setSphereGlyphSize(.15);
    fluidMesh->setRenderMaterial(material1);

    auto deformableObj = std::make_shared<PbdObject>("Dragon");
    deformableObj->setVisualGeometry(fluidMesh);
    deformableObj->setCollidingGeometry(fluidMesh);
    deformableObj->setPhysicsGeometry(fluidMesh);

    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->setModelGeometry(fluidMesh);
    pbdModel->configure(/*Number of Constraints*/ 1,
        /*Constraint configuration*/ "ConstantDensity 1.0 0.3",
        /*Mass*/ 1.0,
        /*Gravity*/ "0 -9.8 0",
        /*TimeStep*/ 0.005,
        /*FixedPoint*/ "",
        /*NumberOfIterationInConstraintSolver*/ 2,
        /*Proximity*/ 0.1,
        /*Contact stiffness*/ 1.0);
    deformableObj->setDynamicalModel(pbdModel);

    auto pbdSolver = std::make_shared<PbdSolver>();
    pbdSolver->setPbdObject(deformableObj);
    scene->addNonlinearSolver(pbdSolver);

    scene->addSceneObject(deformableObj);

    // box
    StdVectorOfVec3d vertList;
    int nSides = 5;
    double width = 40.0;
    double height = 40.0;
    int nRows = 2;
    int nCols = 2;
    vertList.resize(nRows*nCols*nSides);
    const double dy = width / (double)(nCols - 1);
    const double dx = height / (double)(nRows - 1);
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            const double y = (double)dy*j;
            const double x = (double)dx*i;
            vertList[i*nCols + j] = Vec3d(x - 20, -10.0, y - 20);
        }
    }

    // c. Add connectivity data
    std::vector<SurfaceMesh::TriangleArray> triangles;
    for (std::size_t i = 0; i < nRows - 1; ++i)
    {
        for (std::size_t j = 0; j < nCols - 1; j++)
        {
            SurfaceMesh::TriangleArray tri[2];
            tri[0] = { { i*nCols + j, i*nCols + j + 1, (i + 1)*nCols + j } };
            tri[1] = { { (i + 1)*nCols + j + 1, (i + 1)*nCols + j, i*nCols + j + 1 } };
            triangles.push_back(tri[0]);
            triangles.push_back(tri[1]);
        }
    }

    int nPointPerSide = nRows * nCols;
    //sidewalls 1 and 2 of box
    width = 10.0;
    height = 40.0;
    nRows = 2;
    nCols = 2;
    const double dz = width / (double)(nCols - 1);
    const double dx1 = height / (double)(nRows - 1);
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            const double z = (double)dz*j;
            const double x = (double)dx1*i;
            vertList[(nPointPerSide)+i*nCols + j] = Vec3d(x - 20, z - 10.0, 20);
            vertList[(nPointPerSide*2)+i*nCols + j] = Vec3d(x - 20, z - 10.0, -20);
        }
    }

    // c. Add connectivity data
    for (std::size_t i = 0; i < nRows - 1; ++i)
    {
        for (std::size_t j = 0; j < nCols - 1; j++)
        {
            SurfaceMesh::TriangleArray tri[2];
            tri[0] = { { (nPointPerSide)+i*nCols + j, (nPointPerSide)+i*nCols + j + 1, (nPointPerSide)+(i + 1)*nCols + j } };
            tri[1] = { { (nPointPerSide)+(i + 1)*nCols + j + 1, (nPointPerSide)+(i + 1)*nCols + j, (nPointPerSide)+i*nCols + j + 1 } };
            triangles.push_back(tri[0]);
            triangles.push_back(tri[1]);
            tri[0] = { { (nPointPerSide*2)+i*nCols + j, (nPointPerSide*2)+i*nCols + j + 1, (nPointPerSide*2)+(i + 1)*nCols + j } };
            tri[1] = { { (nPointPerSide*2)+(i + 1)*nCols + j + 1, (nPointPerSide*2)+(i + 1)*nCols + j, (nPointPerSide*2)+i*nCols + j + 1 } };
            triangles.push_back(tri[0]);
            triangles.push_back(tri[1]);
        }
    }

    //sidewalls 3 and 4 of box
    width = 10.0;
    height = 40.0;
    nRows = 2;
    nCols = 2;
    const double dz1 = width / (double)(nCols - 1);
    const double dy1 = height / (double)(nRows - 1);
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            const double z = (double)dz1*j;
            const double y = (double)dy1*i;
            vertList[(nPointPerSide * 3)+i*nCols + j] = Vec3d(20, z - 10.0, y-20);
            vertList[(nPointPerSide * 4) + i*nCols + j] = Vec3d(-20, z - 10.0, y-20);
        }
    }

    // c. Add connectivity data
    for (std::size_t i = 0; i < nRows - 1; ++i)
    {
        for (std::size_t j = 0; j < nCols - 1; j++)
        {
            SurfaceMesh::TriangleArray tri[2];
            tri[0] = { { (nPointPerSide * 3)+i*nCols + j, (nPointPerSide * 3)+i*nCols + j + 1, (nPointPerSide * 3)+(i + 1)*nCols + j } };
            tri[1] = { { (nPointPerSide * 3)+(i + 1)*nCols + j + 1, (nPointPerSide * 3)+(i + 1)*nCols + j, (nPointPerSide * 3)+i*nCols + j + 1 } };
            triangles.push_back(tri[0]);
            triangles.push_back(tri[1]);
            tri[0] = { { (nPointPerSide * 4) + i*nCols + j, (nPointPerSide * 4) + i*nCols + j + 1, (nPointPerSide * 4) + (i + 1)*nCols + j } };
            tri[1] = { { (nPointPerSide * 4) + (i + 1)*nCols + j + 1, (nPointPerSide * 4) + (i + 1)*nCols + j, (nPointPerSide * 4) + i*nCols + j + 1 } };
            triangles.push_back(tri[0]);
            triangles.push_back(tri[1]);
        }
    }

    auto floorMeshColliding = std::make_shared<SurfaceMesh>();
    floorMeshColliding->initialize(vertList, triangles);
    auto floorMeshVisual = std::make_shared<SurfaceMesh>();
    floorMeshVisual->initialize(vertList, triangles);
    auto floorMeshPhysics = std::make_shared<SurfaceMesh>();
    floorMeshPhysics->initialize(vertList, triangles);


    auto floorMapP2V = std::make_shared<OneToOneMap>();
    floorMapP2V->setMaster(floorMeshPhysics);
    floorMapP2V->setSlave(floorMeshVisual);
    floorMapP2V->compute();


    auto floorMapP2C = std::make_shared<OneToOneMap>();
    floorMapP2C->setMaster(floorMeshPhysics);
    floorMapP2C->setSlave(floorMeshColliding);
    floorMapP2C->compute();

    auto floorMapC2V = std::make_shared<OneToOneMap>();
    floorMapC2V->setMaster(floorMeshColliding);
    floorMapC2V->setSlave(floorMeshVisual);
    floorMapC2V->compute();

    auto floor = std::make_shared<PbdObject>("Floor");
    floor->setCollidingGeometry(floorMeshColliding);
    floor->setVisualGeometry(floorMeshVisual);
    floor->setPhysicsGeometry(floorMeshPhysics);
    floor->setPhysicsToCollidingMap(floorMapP2C);
    floor->setPhysicsToVisualMap(floorMapP2V);
    floor->setCollidingToVisualMap(floorMapC2V);

    auto pbdModel2 = std::make_shared<PbdModel>();
    pbdModel2->setModelGeometry(floorMeshPhysics);
    pbdModel2->configure(/*Number of Constraints*/ 0,
        /*Mass*/ 0.0,
        /*Proximity*/ 0.1,
        /*Contact stiffness*/ 1.0);
    floor->setDynamicalModel(pbdModel2);

    auto pbdSolverfloor = std::make_shared<PbdSolver>();
    pbdSolverfloor->setPbdObject(floor);
    scene->addNonlinearSolver(pbdSolverfloor);

    scene->addSceneObject(floor);

    // Collisions
    auto colGraph = scene->getCollisionGraph();
    auto pair = std::make_shared<PbdInteractionPair>(PbdInteractionPair(deformableObj, floor));
    pair->setNumberOfInterations(2);

    colGraph->addInteractionPair(pair);

    // Light (white)
    auto whiteLight = std::make_shared<DirectionalLight>("whiteLight");
    whiteLight->setFocalPoint(Vec3d(5, -8, -5));
    whiteLight->setIntensity(7);
    scene->addLight(whiteLight);

    // print UPS
    auto ups = std::make_shared<UPSCounter>();
    apiutils::printUPS(sdk->getSceneManager(scene), ups);

    sdk->setActiveScene(scene);
    sdk->startSimulation(SimulationStatus::PAUSED);
}

int main()
{    
    testPbdFluid();
    return 0;
}
