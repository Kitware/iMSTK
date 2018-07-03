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

#include "imstkSimulationManager.h"
#include "imstkMeshIO.h"
#include "imstkPbdObject.h"
#include "imstkPbdSolver.h"
#include "imstkOneToOneMap.h"
#include "imstkAPIUtilities.h"

using namespace imstk;

void testPbdFluidBenchmarking()
{
    std::vector<int> nPointsList = { 5, 10, 20 };
    std::vector<int> cubeSizeList = { 1, 1, 2 };

    int nPointsPerSide = 10;
    double cubeLength = 1;

    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("PBDFluidBenchmarking");

    scene->getCamera()->setPosition(0, 10.0, 25.0);

    //Create PointSet
    StdVectorOfVec3d vertList;
    int nPoints = pow(nPointsPerSide, 3);
    const double spacing = cubeLength / nPointsPerSide;

    vertList.resize(nPoints);
    for (int i = 0; i < nPointsPerSide; ++i)
    {
        for (int j = 0; j < nPointsPerSide; j++)
        {
            for (int k = 0; k < nPointsPerSide; k++)
            {
                vertList[i*nPointsPerSide*nPointsPerSide + j*nPointsPerSide + k] =
                    Vec3d((double)i * spacing, (double)j * spacing, (double)k * spacing);
            }
        }
    }

    auto cubeMesh = std::make_shared<PointSet>();
    cubeMesh->initialize(vertList);

    auto material = std::make_shared<RenderMaterial>();
    material->setDiffuseColor(Color::Blue);
    material->setSphereGlyphSize(.1);
    cubeMesh->setRenderMaterial(material);

    auto cube = std::make_shared<PbdObject>("Cube");
    cube->setCollidingGeometry(cubeMesh);
    cube->setVisualGeometry(cubeMesh);
    cube->setPhysicsGeometry(cubeMesh);

    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->setModelGeometry(cubeMesh);
    pbdModel->configure(/*Number of Constraints*/ 1,
        /*Constraint configuration*/ "ConstantDensity 1.0 0.3",
        /*Mass*/ 1.0,
        /*Gravity*/ "0 -9.8 0",
        /*TimeStep*/ 0.005,
        /*FixedPoint*/ "",
        /*NumberOfIterationInConstraintSolver*/ 2,
        /*Proximity*/ 0.2,
        /*Contact stiffness*/ 1.0);
    cube->setDynamicalModel(pbdModel);

    auto pbdSolver = std::make_shared<PbdSolver>();
    pbdSolver->setPbdObject(cube);
    scene->addNonlinearSolver(pbdSolver);

    scene->addSceneObject(cube);

    // plane
    double width = 40.0;
    double height = 40.0;
    int nRows = 2;
    int nCols = 2;
    vertList.resize(nRows*nCols);
    const double dy = width / (double)(nCols - 1);
    const double dx = height / (double)(nRows - 1);
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            const double y = (double)dy*j;
            const double x = (double)dx*i;
            vertList[i*nCols + j] = Vec3d(x - 20, -0.5, y - 20);
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
    auto pair = std::make_shared<PbdInteractionPair>(PbdInteractionPair(cube, floor));
    pair->setNumberOfInterations(2);

    auto dynaModel1 = std::static_pointer_cast<PbdModel>(cube->getDynamicalModel());

    colGraph->addInteractionPair(pair);

    // print UPS
    auto ups = std::make_shared<UPSCounter>();
    apiutils::printUPS(sdk->getSceneManager(scene), ups);

    // Light (white)
    auto whiteLight = std::make_shared<DirectionalLight>("whiteLight");
    whiteLight->setFocalPoint(Vec3d(5, -8, -5));
    whiteLight->setIntensity(7);
    scene->addLight(whiteLight);

    scene->getCamera()->setPosition(0, 10.0, 10.0);

    sdk->setActiveScene(scene);
    sdk->startSimulation(SimulationStatus::PAUSED);
}

int main()
{
    
    testPbdFluidBenchmarking();
    
    return 0;
}
