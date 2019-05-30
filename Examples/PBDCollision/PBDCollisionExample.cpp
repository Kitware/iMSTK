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
#include "imstkPbdObject.h"
#include "imstkPbdSolver.h"
#include "imstkTetrahedralMesh.h"
#include "imstkMeshIO.h"
#include "imstkOneToOneMap.h"
#include "imstkMeshToMeshBruteForceCD.h"
#include "imstkPBDCollisionHandling.h"

using namespace imstk;

///
/// \brief This example demonstrates the collision interaction
/// using Position based dynamics
///
int main()
{
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("PbdCollision");

    scene->getCamera()->setPosition(0, 10.0, 10.0);

    // Load a sample mesh
    auto tetMesh = MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");
    if (!tetMesh)
    {
        LOG(WARNING) << "Could not read mesh from file.";
        return 1;
    }

    auto surfMesh = std::make_shared<SurfaceMesh>();
    auto surfMeshVisual = std::make_shared<SurfaceMesh>();
    auto volTetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(tetMesh);
    if (!volTetMesh)
    {
        LOG(WARNING) << "Dynamic pointer cast from PointSet to TetrahedralMesh failed!";
        return 1;
    }
    volTetMesh->extractSurfaceMesh(surfMesh, true);

    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::WIREFRAME_SURFACE);
    auto surfMeshModel = std::make_shared<VisualModel>(surfMesh);
    surfMeshModel->setRenderMaterial(material);

    auto deformMapP2V = std::make_shared<OneToOneMap>();
    deformMapP2V->setMaster(tetMesh);
    deformMapP2V->setSlave(surfMesh);
    deformMapP2V->compute();

    auto deformMapC2V = std::make_shared<OneToOneMap>();
    deformMapC2V->setMaster(surfMesh);
    deformMapC2V->setSlave(surfMesh);
    deformMapC2V->compute();

    auto deformMapP2C = std::make_shared<OneToOneMap>();
    deformMapP2C->setMaster(tetMesh);
    deformMapP2C->setSlave(surfMesh);
    deformMapP2C->compute();

    auto deformableObj = std::make_shared<PbdObject>("Dragon");
    deformableObj->addVisualModel(surfMeshModel);
    deformableObj->setCollidingGeometry(surfMesh);
    deformableObj->setPhysicsGeometry(volTetMesh);
    deformableObj->setPhysicsToCollidingMap(deformMapP2C);
    deformableObj->setPhysicsToVisualMap(deformMapP2V);
    deformableObj->setCollidingToVisualMap(deformMapC2V);

    // Create model and object
    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->setModelGeometry(volTetMesh);
    pbdModel->configure(/*Number of Constraints*/ 1,
        /*Constraint configuration*/ "FEM NeoHookean 1.0 0.3",
        /*Mass*/ 1.0,
        /*Gravity*/ "0 -9.8 0",
        /*TimeStep*/ 0.001,
        /*FixedPoint*/ "",
        /*NumberOfIterationInConstraintSolver*/ 2,
        /*Proximity*/ 0.1,
        /*Contact stiffness*/ 0.01);
    deformableObj->setDynamicalModel(pbdModel);

    // Create solver
    auto pbdSolver = std::make_shared<PbdSolver>();
    pbdSolver->setPbdObject(deformableObj);
    scene->addNonlinearSolver(pbdSolver);

    scene->addSceneObject(deformableObj);

    bool clothTest = 0;
    bool volumetric = !clothTest;

    // Build floor geometry
    StdVectorOfVec3d vertList;
    double width = 100.0;
    double height = 100.0;
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
            vertList[i*nCols + j] = Vec3d(x - 50, -10.0, y - 50);
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
    auto floorMesh = std::make_shared<SurfaceMesh>();
    floorMesh->initialize(vertList, triangles);

    auto materialFloor = std::make_shared<RenderMaterial>();
    materialFloor->setDisplayMode(RenderMaterial::DisplayMode::WIREFRAME_SURFACE);
    auto floorMeshModel = std::make_shared<VisualModel>(floorMesh);
    floorMeshModel->setRenderMaterial(materialFloor);

    auto floorMapP2V = std::make_shared<OneToOneMap>();
    floorMapP2V->setMaster(floorMesh);
    floorMapP2V->setSlave(floorMesh);
    floorMapP2V->compute();

    auto floorMapP2C = std::make_shared<OneToOneMap>();
    floorMapP2C->setMaster(floorMesh);
    floorMapP2C->setSlave(floorMesh);
    floorMapP2C->compute();

    auto floorMapC2V = std::make_shared<OneToOneMap>();
    floorMapC2V->setMaster(floorMesh);
    floorMapC2V->setSlave(floorMesh);
    floorMapC2V->compute();

    auto floor = std::make_shared<PbdObject>("Floor");
    floor->setCollidingGeometry(floorMesh);
    floor->setVisualGeometry(floorMesh);
    floor->setPhysicsGeometry(floorMesh);
    floor->setPhysicsToCollidingMap(floorMapP2C);
    floor->setPhysicsToVisualMap(floorMapP2V);
    floor->setCollidingToVisualMap(floorMapC2V);

    auto pbdModel2 = std::make_shared<PbdModel>();
    pbdModel2->setModelGeometry(floorMesh);
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

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    sdk->setActiveScene(scene);
    sdk->startSimulation(SimulationStatus::PAUSED);

    return 0;
}
