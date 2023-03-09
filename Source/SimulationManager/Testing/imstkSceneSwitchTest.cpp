/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkGeometryUtilities.h"
#include "imstkMeshIO.h"
#include "imstkPbdSystem.h"
#include "imstkPbdSystemConfig.h"
#include "imstkPointwiseMap.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneUtils.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

#include <gtest/gtest.h>

using namespace imstk;

std::shared_ptr<Scene>
createSoftBodyScene(std::string sceneName)
{
    auto scene = std::make_shared<Scene>(sceneName);
    scene->getActiveCamera()->setPosition(0.0, 2.0, 15.0);

    // Load a sample mesh
    auto tetMesh = MeshIO::read<TetrahedralMesh>(iMSTK_DATA_ROOT "/asianDragon/asianDragon.vtu");

    // Extract the surface mesh
    std::shared_ptr<SurfaceMesh> surfMesh = tetMesh->extractSurfaceMesh();

    auto pbdSystem = std::make_shared<PbdSystem>();

    // Configure model
    auto pbdConfig = std::make_shared<PbdSystemConfig>();
    pbdConfig->m_secParams->m_YoungModulus = 1000000.0;
    pbdConfig->m_secParams->m_PoissonRatio = 0.3;
    pbdConfig->enableStrainEnergyConstraint(PbdStrainEnergyConstraint::MaterialType::StVK);
    pbdConfig->m_gravity    = Vec3d(0.0, -9.8, 0.0);
    pbdConfig->m_iterations = 5;
    pbdConfig->m_dt = 0.03;
    pbdSystem->configure(pbdConfig);

    auto pbdObj = SceneUtils::makePbdEntity("PbdObj", surfMesh, nullptr, tetMesh, pbdSystem);
    // Set the geometries
    pbdObj->getComponent<VisualModel>()->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    auto method = pbdObj->getComponent<PbdMethod>();
    method->setPhysicsToVisualMap(std::make_shared<PointwiseMap>(tetMesh, surfMesh));
    method->getPbdBody()->uniformMassValue = 1.0;
    method->getPbdBody()->fixedNodeIds     = { 51, 127, 178 };

    scene->addSceneObject(pbdObj);

    return scene;
}

std::shared_ptr<Scene>
createClothScene(std::string sceneName)
{
    // Setup the Geometry
    Vec2d                        size      = Vec2d(10.0, 10.0);
    Vec2i                        dim       = Vec2i(4, 4);
    std::shared_ptr<SurfaceMesh> clothMesh =
        GeometryUtils::toTriangleGrid(Vec3d::Zero(), size, dim);

    // Setup the Parameters
    auto pbdConfig = std::make_shared<PbdSystemConfig>();
    pbdConfig->enableConstraint(PbdSystemConfig::ConstraintGenType::Distance, 1.0e2);
    pbdConfig->enableConstraint(PbdSystemConfig::ConstraintGenType::Dihedral, 1.0e1);
    pbdConfig->m_gravity    = Vec3d(0.0, -9.8, 0.0);
    pbdConfig->m_dt         = 0.03;
    pbdConfig->m_iterations = 5;

    // Setup the Model
    auto pbdSystem = std::make_shared<PbdSystem>();
    pbdSystem->configure(pbdConfig);

    auto clothObj = SceneUtils::makePbdEntity("Cloth", clothMesh, nullptr, clothMesh, pbdSystem);
    // Setup the Object
    auto renderMat = clothObj->getComponent<VisualModel>()->getRenderMaterial();
    renderMat->setBackFaceCulling(false);
    renderMat->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    auto pbdBody = clothObj->getComponent<PbdMethod>()->getPbdBody();
    pbdBody->fixedNodeIds     = { 0, dim[0] - 1 };
    pbdBody->uniformMassValue = size[0] * size[1] / static_cast<double>(dim[0] * dim[1]);

    auto scene = std::make_shared<Scene>("PBDCloth");
    scene->addSceneObject(clothObj);
    scene->getActiveCamera()->setFocalPoint(0.0, -5.0, 5.0);
    scene->getActiveCamera()->setPosition(-15.0, -5.0, 25.0);
    return scene;
}

///
/// \brief Test we are able to switch scenes while rendering
///
TEST(imstkSimulationManagerTest, TestSceneSwitchWithRendering)
{
    // Simulation manager defaults to rendering mode
    std::shared_ptr<Scene> scene1 = createClothScene("clothScene");
    std::shared_ptr<Scene> scene2 = createSoftBodyScene("deformableBodyScene");

    scene1->initialize();
    scene2->initialize();

    // Setup a viewer to render in its own thread
    auto viewer = std::make_shared<VTKViewer>("Viewer");
    viewer->setActiveScene(scene1);

    auto sceneManager = std::make_shared<SceneManager>();
    sceneManager->setActiveScene(scene1);

    auto driver = std::make_shared<SimulationManager>();
    driver->addModule(viewer);
    driver->addModule(sceneManager);
    driver->setDesiredDt(0.03);

    // After 4s switch scenes, after 8s switch back, after 12s stop
    double elapsedTime = 0.0;
    connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
        {
            elapsedTime += sceneManager->getDt();
            if (elapsedTime > 3.0)
            {
                driver->requestStatus(ModuleDriverStopped);
            }
            else if (elapsedTime > 2.0)
            {
                if (sceneManager->getActiveScene() != scene1)
                {
                    LOG(INFO) << "Switching to scene1";
                    sceneManager->setActiveScene(scene1);
                    viewer->setActiveScene(scene1);
                }
            }
            else if (elapsedTime > 1.0)
            {
                if (sceneManager->getActiveScene() != scene2)
                {
                    LOG(INFO) << "Switching to scene2";
                    sceneManager->setActiveScene(scene2);
                    viewer->setActiveScene(scene2);
                }
            }
        });

    driver->start();
}

///
/// \brief Test we are able to switch scenes without rendering
///
TEST(imstkSimulationManagerTest, TestSceneSwitchWithoutRendering)
{
    // Simulation manager defaults to rendering mode
    std::shared_ptr<Scene> scene1 = createClothScene("clothScene");
    std::shared_ptr<Scene> scene2 = createSoftBodyScene("deformableBodyScene");

    scene1->initialize();
    scene2->initialize();

    auto sceneManager = std::make_shared<SceneManager>();
    sceneManager->setActiveScene(scene1);

    auto driver = std::make_shared<SimulationManager>();
    driver->addModule(sceneManager);
    driver->setDesiredDt(0.03);

    // After 4s switch scenes, after 8s switch back, after 12s stop
    double elapsedTime = 0.0;
    connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
        {
            elapsedTime += sceneManager->getDt();
            if (elapsedTime > 3.0)
            {
                driver->requestStatus(ModuleDriverStopped);
            }
            else if (elapsedTime > 2.0)
            {
                if (sceneManager->getActiveScene() != scene1)
                {
                    LOG(INFO) << "Switching to scene1";
                    sceneManager->setActiveScene(scene1);
                }
            }
            else if (elapsedTime > 1.0)
            {
                if (sceneManager->getActiveScene() != scene2)
                {
                    LOG(INFO) << "Switching to scene2";
                    sceneManager->setActiveScene(scene2);
                }
            }
        });

    driver->start();
}