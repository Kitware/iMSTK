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
#include "imstkMeshToMeshBruteforceCD.h"
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

    // dragon
    auto tetMesh = MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");
    if (!tetMesh)
    {
        LOG(WARNING) << "Could not read mesh from file.";
        return 1;
    }
    //tetMesh->scale(5., Geometry::TransformType::ApplyToData);
    //tetMesh->translate(Vec3d(0., -5., -5.) , Geometry::TransformType::ApplyToData);

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
    auto surfMeshVisualModel = std::make_shared<VisualModel>(surfMesh);
    surfMeshVisualModel->setRenderMaterial(material);

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
    //deformableObj->setVisualGeometry(surfMesh);
    deformableObj->addVisualModel(surfMeshVisualModel);
    deformableObj->setCollidingGeometry(surfMesh);
    deformableObj->setPhysicsGeometry(volTetMesh);
    deformableObj->setPhysicsToCollidingMap(deformMapP2C);
    deformableObj->setPhysicsToVisualMap(deformMapP2V);
    deformableObj->setCollidingToVisualMap(deformMapC2V);

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

    auto pbdSolver = std::make_shared<PbdSolver>();
    pbdSolver->setPbdObject(deformableObj);
    scene->addNonlinearSolver(pbdSolver);

    scene->addSceneObject(deformableObj);

    bool clothTest = 0;
    bool volumetric = !clothTest;
    if (clothTest)
    {
        auto clothMesh = std::make_shared<SurfaceMesh>();
        StdVectorOfVec3d vertList;
        double width = 60.0;
        double height = 60.0;
        int nRows = 10;
        int nCols = 10;
        int corner[4] = { 1, nRows, nRows * nCols - nCols + 1, nRows * nCols };
        char intStr[33];
        std::string fixed_corner;
        for (unsigned int i = 0; i < 4; ++i)
        {
            std::sprintf(intStr, "%d", corner[i]);
            fixed_corner += std::string(intStr) + ' ';
        }
        vertList.resize(nRows * nCols);
        const double dy = width / (double)(nCols - 1);
        const double dx = height / (double)(nRows - 1);
        for (int i = 0; i < nRows; ++i)
        {
            for (int j = 0; j < nCols; j++)
            {
                const double y = (double)dy * j;
                const double x = (double)dx * i;
                vertList[i * nCols + j] = Vec3d(x - 30, -10, y - 30);
            }
        }
        clothMesh->setInitialVertexPositions(vertList);
        clothMesh->setVertexPositions(vertList);

        // c. Add connectivity data
        std::vector<SurfaceMesh::TriangleArray> triangles;
        for (std::size_t i = 0; i < nRows - 1; ++i)
        {
            for (std::size_t j = 0; j < nCols - 1; j++)
            {
                SurfaceMesh::TriangleArray tri[2];
                tri[0] = { { i*nCols + j, i*nCols + j + 1, (i + 1) * nCols + j } };
                tri[1] = { { (i + 1) * nCols + j + 1, (i + 1) * nCols + j, i * nCols + j + 1 } };
                triangles.push_back(tri[0]);
                triangles.push_back(tri[1]);
            }
        }
        clothMesh->setTrianglesVertices(triangles);

        auto oneToOneFloor = std::make_shared<OneToOneMap>();
        oneToOneFloor->setMaster(clothMesh);
        oneToOneFloor->setSlave(clothMesh);
        oneToOneFloor->compute();

        auto floor = std::make_shared<PbdObject>("Floor");
        floor->setCollidingGeometry(clothMesh);
        floor->setVisualGeometry(clothMesh);
        floor->setPhysicsGeometry(clothMesh);
        floor->setPhysicsToCollidingMap(oneToOneFloor);
        floor->setPhysicsToVisualMap(oneToOneFloor);
        //floor->setCollidingToVisualMap(oneToOneFloor);
        //floor->initialize(/*Number of constraints*/ 2,
        //                  /*Constraint configuration*/ "Distance 0.1",
        //                  /*Constraint configuration*/ "Dihedral 0.001",
        //                  /*Mass*/ 0.1,
        //                  /*Gravity*/ "0 9.8 0",
        //                  /*TimeStep*/ 0.002,
        //                  /*FixedPoint*/ fixed_corner.c_str(),
        //                  /*NumberOfIterationInConstraintSolver*/ 5,
        //                  /*Proximity*/ 0.1,
        //                  /*Contact stiffness*/ 0.95);
        scene->addSceneObject(floor);

        std::cout << "nbr of vertices in cloth mesh" << clothMesh->getNumVertices() << std::endl;

        // Collisions
        auto clothTestcolGraph = scene->getCollisionGraph();
        auto pair1 = std::make_shared<PbdInteractionPair>(PbdInteractionPair(deformableObj, floor));
        pair1->setNumberOfInterations(5);

        clothTestcolGraph->addInteractionPair(pair1);

        scene->getCamera()->setPosition(0, 0, 50);
    }
    else if (0)
    {
        auto tetMesh1 = MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");
        if (!tetMesh1)
        {
            LOG(WARNING) << "Could not read mesh from file.";
            return 1;
        }

        auto surfMesh1 = std::make_shared<SurfaceMesh>();
        auto surfMeshVisual1 = std::make_shared<SurfaceMesh>();
        auto volTetMesh1 = std::dynamic_pointer_cast<TetrahedralMesh>(tetMesh1);
        if (!volTetMesh1)
        {
            LOG(WARNING) << "Dynamic pointer cast from PointSet to TetrahedralMesh failed!";
            return 1;
        }

        auto vs = volTetMesh1->getInitialVertexPositions();
        Vec3d tmpPos;
        for (int i = 0; i < volTetMesh1->getNumVertices(); ++i)
        {
            tmpPos = volTetMesh1->getVertexPosition(i);
            tmpPos[1] -= 6;
            volTetMesh1->setVertexPosition(i, tmpPos);
        }
        volTetMesh1->setInitialVertexPositions(volTetMesh1->getVertexPositions());

        volTetMesh1->extractSurfaceMesh(surfMesh1);
        volTetMesh1->extractSurfaceMesh(surfMeshVisual1);

        auto deformMapP2V1 = std::make_shared<OneToOneMap>();
        deformMapP2V1->setMaster(volTetMesh1);
        deformMapP2V1->setSlave(surfMeshVisual1);
        deformMapP2V1->compute();

        auto deformMapC2V1 = std::make_shared<OneToOneMap>();
        deformMapC2V1->setMaster(surfMesh1);
        deformMapC2V1->setSlave(surfMeshVisual1);
        deformMapC2V1->compute();

        auto deformMapP2C1 = std::make_shared<OneToOneMap>();
        deformMapP2C1->setMaster(volTetMesh1);
        deformMapP2C1->setSlave(surfMesh1);
        deformMapP2C1->compute();

        auto deformableObj1 = std::make_shared<PbdObject>("Dragon2");
        deformableObj1->setVisualGeometry(surfMeshVisual1);
        deformableObj1->setCollidingGeometry(surfMesh1);
        deformableObj1->setPhysicsGeometry(volTetMesh1);
        deformableObj1->setPhysicsToCollidingMap(deformMapP2C1);
        deformableObj1->setPhysicsToVisualMap(deformMapP2V1);
        deformableObj1->setCollidingToVisualMap(deformMapC2V1);
        //deformableObj1->initialize(/*Number of Constraints*/ 1,
        //                           /*Constraint configuration*/ "FEM NeoHookean 10.0 0.5",
        //                           /*Mass*/ 0.0,
        //                           /*Gravity*/ "0 -9.8 0",
        //                           /*TimeStep*/ 0.002,
        //                           /*FixedPoint*/ "",
        //                           /*NumberOfIterationInConstraintSolver*/ 2,
        //                           /*Proximity*/ 0.1,
        //                           /*Contact stiffness*/ 0.01);

        scene->addSceneObject(deformableObj1);

        // Collisions
        auto colGraph = scene->getCollisionGraph();
        auto pair = std::make_shared<PbdInteractionPair>(PbdInteractionPair(deformableObj, deformableObj1));
        pair->setNumberOfInterations(2);

        colGraph->addInteractionPair(pair);
    }
    else
    {
        // Build floor geometry
        StdVectorOfVec3d vertList;
        const double width = 100.0;
        const double height = 100.0;
        const int nRows = 2;
        const int nCols = 2;
        vertList.resize(nRows * nCols);
        const double dy = width / (double)(nCols - 1);
        const double dx = height / (double)(nRows - 1);
        for (int i = 0; i < nRows; ++i)
        {
            for (int j = 0; j < nCols; j++)
            {
                const double y = (double)dy * j;
                const double x = (double)dx * i;
                vertList[i * nCols + j] = Vec3d(x - 50, -10.0, y - 50);
            }
        }

        // c. Add connectivity data
        std::vector<SurfaceMesh::TriangleArray> triangles;
        for (std::size_t i = 0; i < nRows - 1; ++i)
        {
            for (std::size_t j = 0; j < nCols - 1; j++)
            {
                SurfaceMesh::TriangleArray tri[2];
                tri[0] = { { i*nCols + j, i*nCols + j + 1, (i + 1) * nCols + j } };
                tri[1] = { { (i + 1) * nCols + j + 1, (i + 1) * nCols + j, i * nCols + j + 1 } };
                triangles.push_back(tri[0]);
                triangles.push_back(tri[1]);
            }
        }
        auto floorMesh = std::make_shared<SurfaceMesh>();
        floorMesh->initialize(vertList, triangles);

        auto materialFloor = std::make_shared<RenderMaterial>();
        materialFloor->setDisplayMode(RenderMaterial::DisplayMode::WIREFRAME_SURFACE);
        auto floorMeshVisual = std::make_shared<VisualModel>(floorMesh);
        floorMeshVisual->setRenderMaterial(materialFloor);

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
        floor->addVisualModel(floorMeshVisual);
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

        auto colData = std::make_shared<CollisionData>();
        auto CD = std::make_shared<MeshToMeshBruteForceCD>(surfMesh, floorMesh, colData);

        auto CH = std::make_shared<PBDCollisionHandling>(CollisionHandling::Side::A,
                        CD->getCollisionData(), deformableObj, floor, pbdSolver);

        scene->getCollisionGraph()->addInteractionPair(deformableObj, floor, CD, CH, nullptr);
    }

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    sdk->setActiveScene(scene);
    sdk->startSimulation(SimulationStatus::RUNNING);

    return 0;
}
