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
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdSolver.h"
#include "imstkAPIUtilities.h"
#include "imstkMeshToMeshBruteForceCD.h"
#include "imstkPBDCollisionHandling.h"

using namespace imstk;

///
/// \brief This example demonstrates the fluids simulation
/// using Position based dynamics
///
int
main()
{
    auto simManager = std::make_shared<SimulationManager>();
    auto scene      = simManager->createNewScene("PBDFluid");

    scene->getCamera()->setPosition(0, 10.0, 15.0);

    // Load a sample mesh
    auto tetMesh = MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");

    auto fluidMesh = std::make_shared<PointSet>();
    fluidMesh->initialize(tetMesh->getInitialVertexPositions());

    auto fluidVisualModel = std::make_shared<VisualModel>(fluidMesh);

    auto material1 = std::make_shared<RenderMaterial>();
    material1->setColor(Color::Blue);
    material1->setSphereGlyphSize(.15);
    fluidVisualModel->setRenderMaterial(material1);

    auto deformableObj = std::make_shared<PbdObject>("Dragon");
    deformableObj->addVisualModel(fluidVisualModel);
    deformableObj->setCollidingGeometry(fluidMesh);
    deformableObj->setPhysicsGeometry(fluidMesh);

    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->setModelGeometry(fluidMesh);

    // Configure model
    auto pbdParams = std::make_shared<PBDModelConfig>();

    // Constant density constraint with stiffness
    pbdParams->enableConstraint(PbdConstraint::Type::ConstantDensity, 1.0);

    // Other parameters
    pbdParams->m_uniformMassValue = 1.0;
    pbdParams->m_gravity          = Vec3d(0, -9.8, 0);
    pbdParams->m_dt               = 0.05;
    pbdParams->m_maxIter          = 2;
    pbdParams->m_proximity        = 0.01;
    pbdParams->m_contactStiffness = 0.2;

    // Set the parameters
    pbdModel->configure(pbdParams);
    pbdModel->setTimeStepSizeType(TimeSteppingType::fixed);
    deformableObj->setDynamicalModel(pbdModel);

    scene->addSceneObject(deformableObj);

    // Create a box to hold the fluid
    StdVectorOfVec3d vertList;
    int              nSides = 5;
    double           width  = 40.0;
    double           height = 40.0;
    int              nRows  = 2;
    int              nCols  = 2;
    vertList.resize(nRows * nCols * nSides);
    const double dy = width / (double)(nCols - 1);
    const double dx = height / (double)(nRows - 1);
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            const double y = (double)dy * j;
            const double x = (double)dx * i;
            vertList[i * nCols + j] = Vec3d(x - 20, -10.0, y - 20);
        }
    }

    // c. Add connectivity data
    std::vector<SurfaceMesh::TriangleArray> triangles;
    for (std::size_t i = 0; i < nRows - 1; ++i)
    {
        for (std::size_t j = 0; j < nCols - 1; j++)
        {
            SurfaceMesh::TriangleArray tri[2];
            tri[0] = { { i* nCols + j, i* nCols + j + 1, (i + 1) * nCols + j } };
            tri[1] = { { (i + 1) * nCols + j + 1, (i + 1) * nCols + j, i * nCols + j + 1 } };
            triangles.push_back(tri[0]);
            triangles.push_back(tri[1]);
        }
    }

    int nPointPerSide = nRows * nCols;
    //sidewalls 1 and 2 of box
    width  = 10.0;
    height = 40.0;
    nRows  = 2;
    nCols  = 2;
    const double dz  = width / (double)(nCols - 1);
    const double dx1 = height / (double)(nRows - 1);
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            const double z = (double)dz * j;
            const double x = (double)dx1 * i;
            vertList[(nPointPerSide) + i * nCols + j]     = Vec3d(x - 20, z - 10.0, 20);
            vertList[(nPointPerSide * 2) + i * nCols + j] = Vec3d(x - 20, z - 10.0, -20);
        }
    }

    // c. Add connectivity data
    for (std::size_t i = 0; i < nRows - 1; ++i)
    {
        for (std::size_t j = 0; j < nCols - 1; j++)
        {
            SurfaceMesh::TriangleArray tri[2];
            tri[0] = { { (nPointPerSide) + i * nCols + j, (nPointPerSide) + i * nCols + j + 1, (nPointPerSide) + (i + 1) * nCols + j } };
            tri[1] = { { (nPointPerSide) + (i + 1) * nCols + j + 1, (nPointPerSide) + (i + 1) * nCols + j, (nPointPerSide) + i * nCols + j + 1 } };
            triangles.push_back(tri[0]);
            triangles.push_back(tri[1]);
            tri[0] = { { (nPointPerSide * 2) + i * nCols + j, (nPointPerSide * 2) + i * nCols + j + 1, (nPointPerSide * 2) + (i + 1) * nCols + j } };
            tri[1] = { { (nPointPerSide * 2) + (i + 1) * nCols + j + 1, (nPointPerSide * 2) + (i + 1) * nCols + j, (nPointPerSide * 2) + i * nCols + j + 1 } };
            triangles.push_back(tri[0]);
            triangles.push_back(tri[1]);
        }
    }

    //sidewalls 3 and 4 of box
    width  = 10.0;
    height = 40.0;
    nRows  = 2;
    nCols  = 2;
    const double dz1 = width / (double)(nCols - 1);
    const double dy1 = height / (double)(nRows - 1);
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            const double z = (double)dz1 * j;
            const double y = (double)dy1 * i;
            vertList[(nPointPerSide * 3) + i * nCols + j] = Vec3d(20, z - 10.0, y - 20);
            vertList[(nPointPerSide * 4) + i * nCols + j] = Vec3d(-20, z - 10.0, y - 20);
        }
    }

    // c. Add connectivity data
    for (std::size_t i = 0; i < nRows - 1; ++i)
    {
        for (std::size_t j = 0; j < nCols - 1; j++)
        {
            SurfaceMesh::TriangleArray tri[2];
            tri[0] = { { (nPointPerSide * 3) + i * nCols + j, (nPointPerSide * 3) + i * nCols + j + 1, (nPointPerSide * 3) + (i + 1) * nCols + j } };
            tri[1] = { { (nPointPerSide * 3) + (i + 1) * nCols + j + 1, (nPointPerSide * 3) + (i + 1) * nCols + j, (nPointPerSide * 3) + i * nCols + j + 1 } };
            triangles.push_back(tri[0]);
            triangles.push_back(tri[1]);
            tri[0] = { { (nPointPerSide * 4) + i * nCols + j, (nPointPerSide * 4) + i * nCols + j + 1, (nPointPerSide * 4) + (i + 1) * nCols + j } };
            tri[1] = { { (nPointPerSide * 4) + (i + 1) * nCols + j + 1, (nPointPerSide * 4) + (i + 1) * nCols + j, (nPointPerSide * 4) + i * nCols + j + 1 } };
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

    auto floor = std::make_shared<PbdObject>("Floor");
    floor->setCollidingGeometry(floorMeshColliding);
    floor->setVisualGeometry(floorMeshVisual);
    floor->setPhysicsGeometry(floorMeshPhysics);

    auto pbdModel2 = std::make_shared<PbdModel>();
    pbdModel2->setModelGeometry(floorMeshPhysics);

    // Configure model
    auto pbdParams2 = std::make_shared<PBDModelConfig>();
    pbdParams2->m_uniformMassValue = 0.0;
    pbdParams2->m_proximity        = 0.1;
    pbdParams2->m_contactStiffness = 1.0;

    pbdModel2->configure(pbdParams2);
    floor->setDynamicalModel(pbdModel2);

    auto pbdSolverfloor = std::make_shared<PbdSolver>();
    pbdSolverfloor->setPbdObject(floor);
    scene->addNonlinearSolver(pbdSolverfloor);

    scene->addSceneObject(floor);

    // Collisions
    auto colGraph = scene->getCollisionGraph();
    auto CD       = std::make_shared<MeshToMeshBruteForceCD>(fluidMesh, floorMeshColliding, nullptr);
    auto CH       = std::make_shared<PBDCollisionHandling>(CollisionHandling::Side::A,
                CD->getCollisionData(), deformableObj, floor);
    colGraph->addInteractionPair(deformableObj, floor, CD, CH, nullptr);

    // Light (white)
    auto whiteLight = std::make_shared<DirectionalLight>("whiteLight");
    whiteLight->setFocalPoint(Vec3d(5, -8, -5));
    whiteLight->setIntensity(7);
    scene->addLight(whiteLight);

    // print UPS
    scene->getConfig()->trackFPS = true;
    apiutils::printUPS(simManager->getSceneManager(scene));

    simManager->setActiveScene(scene);
    simManager->start(SimulationStatus::paused);

    return 0;
}
