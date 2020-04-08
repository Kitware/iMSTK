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
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdSolver.h"
#include "imstkAPIUtilities.h"
#include "imstkOneToOneMap.h"

using namespace imstk;

std::shared_ptr<Scene>
createSoftBodyScene(std::shared_ptr<SimulationManager> simManager, const char* sceneName)
{
    auto sceneConfig = std::make_shared<SceneConfig>();
    sceneConfig->lazyInitialization = true;
    auto scene = simManager->createNewScene(sceneName, sceneConfig);
    scene->getCamera()->setPosition(0, 2.0, 15.0);

    // Load a sample mesh
    auto tetMesh = MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");

    // Extract the surface mesh
    auto surfMesh   = std::make_shared<SurfaceMesh>();
    auto volTetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(tetMesh);
    volTetMesh->extractSurfaceMesh(surfMesh, true);

    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    auto surfMeshModel = std::make_shared<VisualModel>(surfMesh);
    surfMeshModel->setRenderMaterial(material);

    // Construct one to one nodal map based on the above meshes
    auto oneToOneNodalMap = std::make_shared<OneToOneMap>(tetMesh, surfMesh);

    auto deformableObj = std::make_shared<PbdObject>("Dragon");
    auto pbdModel      = std::make_shared<PbdModel>();
    pbdModel->setModelGeometry(volTetMesh);

    // configure model
    auto pbdParams = std::make_shared<PBDModelConfig>();

    // FEM constraint
    pbdParams->m_YoungModulus = 100.0;
    pbdParams->m_PoissonRatio = 0.3;
    pbdParams->m_fixedNodeIds = { 51, 127, 178 };
    pbdParams->enableFEMConstraint(PbdConstraint::Type::FEMTet, PbdFEMConstraint::MaterialType::StVK);

    // Other parameters
    pbdParams->m_uniformMassValue = 1.0;
    pbdParams->m_gravity = Vec3d(0, -9.8, 0);
    pbdParams->m_maxIter = 45;

    // Set the parameters
    pbdModel->configure(pbdParams);
    deformableObj->setDynamicalModel(pbdModel);
    deformableObj->addVisualModel(surfMeshModel);
    deformableObj->setPhysicsGeometry(volTetMesh);
    deformableObj->setPhysicsToVisualMap(oneToOneNodalMap); //assign the computed map

    deformableObj->setDynamicalModel(pbdModel);
    auto pbdSolver = std::make_shared<PbdSolver>();
    pbdSolver->setPbdObject(deformableObj);
    scene->addNonlinearSolver(pbdSolver);

    scene->addSceneObject(deformableObj);

    auto planeGeom = std::make_shared<Plane>();
    planeGeom->setWidth(40);
    planeGeom->setTranslation(0, -6, 0);
    auto planeObj = std::make_shared<CollidingObject>("Plane");
    planeObj->setVisualGeometry(planeGeom);
    planeObj->setCollidingGeometry(planeGeom);
    scene->addSceneObject(planeObj);

    return scene;
}

std::shared_ptr<Scene>
createClothScene(std::shared_ptr<SimulationManager> simManager, const char* sceneName)
{
    auto sceneConfig = std::make_shared<SceneConfig>();
    sceneConfig->lazyInitialization = true;
    auto scene = simManager->createNewScene(sceneName, sceneConfig);

    // Create surface mesh
    auto             surfMesh = std::make_shared<SurfaceMesh>();
    StdVectorOfVec3d vertList;
    const double     width  = 10.0;
    const double     height = 10.0;
    const int        nRows  = 11;
    const int        nCols  = 11;
    vertList.resize(nRows * nCols);
    const double dy = width / (double)(nCols - 1);
    const double dx = height / (double)(nRows - 1);
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            vertList[i * nCols + j] = Vec3d((double)dx * i, 1.0, (double)dy * j);
        }
    }
    surfMesh->setInitialVertexPositions(vertList);
    surfMesh->setVertexPositions(vertList);

    // Add connectivity data
    std::vector<SurfaceMesh::TriangleArray> triangles;
    for (std::size_t i = 0; i < nRows - 1; ++i)
    {
        for (std::size_t j = 0; j < nCols - 1; j++)
        {
            SurfaceMesh::TriangleArray tri[2];
            tri[0] = { { i* nCols + j, (i + 1) * nCols + j, i* nCols + j + 1 } };
            tri[1] = { { (i + 1) * nCols + j + 1, i* nCols + j + 1, (i + 1) * nCols + j } };
            triangles.push_back(tri[0]);
            triangles.push_back(tri[1]);
        }
    }

    surfMesh->setTrianglesVertices(triangles);

    // Create Object & Model
    auto deformableObj = std::make_shared<PbdObject>("Cloth");
    auto pbdModel      = std::make_shared<PbdModel>();
    pbdModel->setModelGeometry(surfMesh);

    // configure model
    auto pbdParams = std::make_shared<PBDModelConfig>();

    // Constraints
    pbdParams->enableConstraint(PbdConstraint::Type::Distance, 0.1);
    pbdParams->enableConstraint(PbdConstraint::Type::Dihedral, 0.001);
    std::vector<size_t> fixedNodes(nCols);
    for (size_t i = 0; i < fixedNodes.size(); i++)
    {
        fixedNodes[i] = i;
    }
    pbdParams->m_fixedNodeIds = fixedNodes;

    // Other parameters
    pbdParams->m_uniformMassValue = 1.0;
    pbdParams->m_gravity   = Vec3d(0, -9.8, 0);
    pbdParams->m_DefaultDt = 0.03;
    pbdParams->m_maxIter   = 5;

    // Set the parameters
    pbdModel->configure(pbdParams);
    deformableObj->setDynamicalModel(pbdModel);
    deformableObj->setPhysicsGeometry(surfMesh);

    auto material = std::make_shared<RenderMaterial>();
    material->setBackFaceCulling(false);
    material->setColor(Color::LightGray);
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    auto surfMeshModel = std::make_shared<VisualModel>(surfMesh);
    surfMeshModel->setRenderMaterial(material);
    deformableObj->addVisualModel(surfMeshModel);

    // Solver
    auto pbdSolver = std::make_shared<PbdSolver>();
    pbdSolver->setPbdObject(deformableObj);
    scene->addNonlinearSolver(pbdSolver);

    scene->addSceneObject(deformableObj);

    scene->getCamera()->setFocalPoint(0, -5, 5);
    scene->getCamera()->setPosition(-15., -5.0, 15.0);

    return scene;
}

void
testMultipleScenesInBackendMode()
{
    auto simConfig = std::make_shared<simManagerConfig>();
    simConfig->simulationMode = SimulationMode::Backend;
    auto simManager = std::make_shared<SimulationManager>(simConfig);

    auto scene1 = createClothScene(simManager, "clothScene");
    auto scene2 = createSoftBodyScene(simManager, "deformableBodyScene");

    // set to scene 1
    simManager->setActiveScene(scene1);

    simManager->initialize();

    simManager->advanceFrame();
    simManager->advanceFrame();
    simManager->advanceFrame();
    simManager->advanceFrame();

    // set to scene 2
    simManager->setActiveScene(scene2);

    simManager->advanceFrame();
    simManager->advanceFrame();
    simManager->advanceFrame();
    simManager->advanceFrame();

    // set to scene 1
    simManager->setActiveScene(scene1);

    simManager->advanceFrame();
    simManager->advanceFrame();
    simManager->advanceFrame();
    simManager->advanceFrame();

    // set to scene 2
    simManager->setActiveScene(scene2);

    simManager->advanceFrame();
    simManager->advanceFrame();
    simManager->advanceFrame();
    simManager->advanceFrame();

    simManager->end();

    std::cout << "Press any key to exit..." << std::endl;

    getchar();
}

void
testMultipleScenesInRenderMode()
{
    // Simulation manager defaults to rendering mode
    auto simManager = std::make_shared<SimulationManager>();
    auto scene1     = createClothScene(simManager, "clothScene");
    auto scene2     = createSoftBodyScene(simManager, "deformableBodyScene");

    scene1->getConfig()->trackFPS = true;

    // set to scene 1
    simManager->setActiveScene(scene2);

    // Create a call back on key press of 's' to switch scenes
    auto viewer = simManager->getViewer();
    if (viewer)
    {
        std::cout << "Press 's/S' to switch scenes" << std::endl;

        viewer->setOnCharFunction('s', [&](InteractorStyle* c) -> bool
        {
            if (simManager->getActiveScene() == scene1)
            {
                simManager->setActiveScene(scene2);
            }
            else
            {
                simManager->setActiveScene(scene1);
            }

            if (!simManager->getActiveScene())
            {
                simManager->setActiveScene(scene1);
            }

            return true;
        });
    }

    if (scene1->getConfig()->trackFPS)
    {
        apiutils::printUPS(simManager->getSceneManager(scene1));
    }

    simManager->start();
}

void
testMultipleScenesInBackgroundMode()
{
    auto simConfig = std::make_shared<simManagerConfig>();
    simConfig->simulationMode = SimulationMode::RunInBackgroundSync;
    auto simManager = std::make_shared<SimulationManager>(simConfig);
    auto scene1     = createClothScene(simManager, "clothScene");
    auto scene2     = createSoftBodyScene(simManager, "deformableBodyScene");

    simManager->setActiveScene(scene1);

    std::cout << "Press 's/S' to switch scenes" << std::endl;
    auto keyPressFunc = [&]()
                        {
                            if (simManager->getActiveScene()->getName() == scene1->getName())
                            {
                                simManager->setActiveScene(scene2);
                            }
                            else
                            {
                                simManager->setActiveScene(scene1);
                            }
                        };
    simManager->addKeyPressCallback(keyPressFunc, 's');

    simManager->start();
}

///
/// \brief Test multiple scenes
///
int
main()
{
    testMultipleScenesInBackendMode();
    testMultipleScenesInBackgroundMode();
    testMultipleScenesInRenderMode();

    return 0;
}
