///*=========================================================================
//
//   Library: iMSTK
//
//   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
//   & Imaging in Medicine, Rensselaer Polytechnic Institute.
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0.txt
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//
//=========================================================================*/

#include "imstkCamera.h"
#include "imstkCapsule.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkHapticDeviceClient.h"
#include "imstkHapticDeviceManager.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLineMesh.h"
#include "imstkLogger.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPbdObjectController.h"
#include "imstkPlane.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSphere.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

#include <thread>
#include <chrono>

using namespace imstk;

///
/// \brief Creates tissue object
///
static std::shared_ptr<PbdObject>
makeTissueObj(const std::string&        name,
              std::shared_ptr<PbdModel> model,
              const double              width,
              const double              height,
              const int                 rowCount,
              const int                 colCount)
{
    // Setup the Geometry
    std::shared_ptr<SurfaceMesh> clothMesh =
        GeometryUtils::toTriangleGrid(Vec3d::Zero(),
            Vec2d(width, height), Vec2i(rowCount, colCount));

    // Setup the Parameters
    model->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 5000.0);
    model->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral, 5000.0);

    // Setup the Model
    model->setModelGeometry(clothMesh);

    // Setup the VisualModel
    auto material = std::make_shared<RenderMaterial>();
    material->setBackFaceCulling(false);
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setShadingModel(RenderMaterial::ShadingModel::PBR);

    auto visualModel = std::make_shared<VisualModel>();
    visualModel->setGeometry(clothMesh);
    visualModel->setRenderMaterial(material);

    // Setup the Object
    auto pbdObject = std::make_shared<PbdObject>(name);
    pbdObject->addVisualModel(visualModel);
    pbdObject->setPhysicsGeometry(clothMesh);
    pbdObject->setCollidingGeometry(clothMesh);
    pbdObject->setDynamicalModel(model);

    pbdObject->getPbdBody()->uniformMassValue = width * height / (rowCount * colCount);
    for (int x = 0; x < rowCount; x++)
    {
        for (int y = 0; y < colCount; y++)
        {
            if (x == 0 || y == 0 || x == rowCount - 1 || y == colCount - 1)
            {
                pbdObject->getPbdBody()->fixedNodeIds.push_back(x * colCount + y);
            }
        }
    }

    return pbdObject;
}

static void
sausageScene()
{
    // Write log to stdout and file
    Logger::startLogger();

    // Setup a scene
    auto scene = std::make_shared<Scene>("SausageScene");
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setPosition(-0.0237419, 0.0368787, 0.338374);
    scene->getActiveCamera()->setViewUp(0.0, 1.0, 0.0);

    auto pbdModel  = std::make_shared<PbdModel>();
    auto pbdConfig = std::make_shared<PbdModelConfig>();
    pbdConfig->m_gravity    = Vec3d(0.0, -20.0, 0.0); // Slightly larger gravity to compensate viscosity
    pbdConfig->m_dt         = 0.005;
    pbdConfig->m_iterations = 3;
    pbdConfig->m_linearDampingCoeff  = 0.03;
    pbdConfig->m_angularDampingCoeff = 0.03;
    pbdConfig->m_doPartitioning      = false;
    pbdModel->configure(pbdConfig);

    // Setup a capsule
    auto rigidObj = std::make_shared<PbdObject>("rigidObj");
    {
        // Setup line geometry
        //auto rigidGeom = std::make_shared<Sphere>(Vec3d(0.0, 0.0, 0.0), 0.5);
        auto rigidGeom = std::make_shared<Capsule>(Vec3d(0.0, 0.1, 0.0), 0.5, 2.0);
        rigidObj->setVisualGeometry(rigidGeom);
        rigidObj->setCollidingGeometry(rigidGeom);
        rigidObj->setPhysicsGeometry(rigidGeom);

        // Setup material
        rigidObj->getVisualModel(0)->getRenderMaterial()->setColor(Color(0.9, 0.0, 0.0));
        rigidObj->getVisualModel(0)->getRenderMaterial()->setShadingModel(RenderMaterial::ShadingModel::PBR);
        rigidObj->getVisualModel(0)->getRenderMaterial()->setRoughness(0.5);
        rigidObj->getVisualModel(0)->getRenderMaterial()->setMetalness(1.0);
        rigidObj->getVisualModel(0)->getRenderMaterial()->setIsDynamicMesh(false);

        rigidObj->setDynamicalModel(pbdModel);

        // Setup body
        rigidObj->getPbdBody()->bodyType    = PbdBody::Type::RIGID;
        rigidObj->getPbdBody()->initPosTest = Vec3d(-4.0, 1.0, 0.0);
        //rigidObj->getPbdBody()->initOrientationTest = Quatd::FromTwoVectors(Vec3d(0.0, 1.0, 0.0), Vec3d(1.0, 0.0, 0.0).normalized());
        rigidObj->getPbdBody()->uniformMassValue = 10.0; // 1.0
        /*rigidObj->getPbdBody()->initInertiaTest = Mat3d::Identity() * 100.0;*/
        rigidObj->getPbdBody()->initInertiaTest = Mat3d::Identity() * 100.0;
    }
    scene->addSceneObject(rigidObj);

    //auto collision = std::make_shared<PbdObjectCollision>(rigidObj, tissueObj, "SurfaceMeshToCapsuleCD");
    ////auto collision = std::make_shared<PbdObjectCollision>(tissueObj, capsuleObj, "SurfaceMeshToSphereCD");
    //scene->addSceneObject(collision);

    // Light
    auto light = std::make_shared<DirectionalLight>();
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight("Light", light);

    // Run the simulation
    {
        // Setup a viewer to render
        auto viewer = std::make_shared<VTKViewer>();
        viewer->setVtkLoggerMode(VTKViewer::VTKLoggerMode::MUTE);
        viewer->setActiveScene(scene);
        viewer->setDebugAxesLength(0.01, 0.01, 0.01);

        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.001);

        // Add mouse and keyboard controls to the viewer
        {
            auto mouseControl = std::make_shared<MouseSceneControl>();
            mouseControl->setDevice(viewer->getMouseDevice());
            mouseControl->setSceneManager(sceneManager);
            viewer->addControl(mouseControl);

            auto keyControl = std::make_shared<KeyboardSceneControl>();
            keyControl->setDevice(viewer->getKeyboardDevice());
            keyControl->setSceneManager(sceneManager);
            keyControl->setModuleDriver(driver);
            viewer->addControl(keyControl);
        }

        driver->start();
    }
}

static void
planeContactScene()
{
    // Write log to stdout and file
    Logger::startLogger();

    // Setup a scene
    auto scene = std::make_shared<Scene>("PlaneContactScene");
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setPosition(-0.0237419, 0.0368787, 0.338374);
    scene->getActiveCamera()->setViewUp(0.0, 1.0, 0.0);

    auto pbdModel  = std::make_shared<PbdModel>();
    auto pbdConfig = std::make_shared<PbdModelConfig>();
    pbdConfig->m_gravity    = Vec3d(0.0, -20.0, 0.0); // Slightly larger gravity to compensate viscosity
    pbdConfig->m_dt         = 0.01;
    pbdConfig->m_iterations = 3;
    pbdConfig->m_linearDampingCoeff  = 0.03;
    pbdConfig->m_angularDampingCoeff = 0.03;
    pbdConfig->m_doPartitioning      = false;
    pbdModel->configure(pbdConfig);

    auto planeObj  = std::make_shared<CollidingObject>("plane");
    auto planeGeom = std::make_shared<Plane>(Vec3d(0.0, 0.0, 0.0), Vec3d(0.0, 1.0, 0.0));
    planeGeom->setWidth(10.0);
    planeObj->setVisualGeometry(planeGeom);
    planeObj->setCollidingGeometry(planeGeom);
    scene->addSceneObject(planeObj);

    // Setup a capsule
    auto rigidPbdObj = std::make_shared<PbdObject>("rigidPbdObj");
    {
        // Setup line geometry
        //auto rigidGeom = std::make_shared<Sphere>(Vec3d(0.0, 0.0, 0.0), 0.5);
        auto                         rigidGeom = std::make_shared<Capsule>(Vec3d(0.0, 0.1, 0.0), 0.5, 2.0);
        std::shared_ptr<SurfaceMesh> surfMesh  = GeometryUtils::toSurfaceMesh(rigidGeom);
        rigidPbdObj->setVisualGeometry(surfMesh);
        rigidPbdObj->setCollidingGeometry(surfMesh);
        rigidPbdObj->setPhysicsGeometry(surfMesh);

        // Setup material
        rigidPbdObj->getVisualModel(0)->getRenderMaterial()->setColor(Color(0.9, 0.0, 0.0));
        rigidPbdObj->getVisualModel(0)->getRenderMaterial()->setShadingModel(RenderMaterial::ShadingModel::PBR);
        rigidPbdObj->getVisualModel(0)->getRenderMaterial()->setRoughness(0.5);
        rigidPbdObj->getVisualModel(0)->getRenderMaterial()->setMetalness(1.0);
        rigidPbdObj->getVisualModel(0)->getRenderMaterial()->setIsDynamicMesh(false);

        rigidPbdObj->setDynamicalModel(pbdModel);

        // Setup body
        rigidPbdObj->getPbdBody()->bodyType    = PbdBody::Type::RIGID;
        rigidPbdObj->getPbdBody()->initPosTest = Vec3d(-4.0, 5.0, 0.0);
        //rigidPbdObj->getPbdBody()->initOrientationTest = Quatd::FromTwoVectors(Vec3d(0.0, 1.0, 0.0), Vec3d(1.0, 0.0, 0.0).normalized());
        rigidPbdObj->getPbdBody()->uniformMassValue = 10.0; // 1.0
        /*rigidPbdObj->getPbdBody()->initInertiaTest = Mat3d::Identity() * 100.0;*/
        rigidPbdObj->getPbdBody()->initInertiaTest = Mat3d::Identity() * 1.0;
    }
    scene->addSceneObject(rigidPbdObj);

    auto collision = std::make_shared<PbdObjectCollision>(rigidPbdObj, planeObj, "PointSetToPlaneCD");
    scene->addSceneObject(collision);

    // Light
    auto light = std::make_shared<DirectionalLight>();
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight("Light", light);

    // Run the simulation
    {
        // Setup a viewer to render
        auto viewer = std::make_shared<VTKViewer>();
        viewer->setVtkLoggerMode(VTKViewer::VTKLoggerMode::MUTE);
        viewer->setActiveScene(scene);
        viewer->setDebugAxesLength(0.01, 0.01, 0.01);

        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.01);

        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            });
        connect<Event>(sceneManager, &SceneManager::preUpdate, [&](Event*)
            {
                rigidPbdObj->getPbdModel()->getConfig()->m_dt = sceneManager->getDt();
            });

        // Add mouse and keyboard controls to the viewer
        {
            auto mouseControl = std::make_shared<MouseSceneControl>();
            mouseControl->setDevice(viewer->getMouseDevice());
            mouseControl->setSceneManager(sceneManager);
            viewer->addControl(mouseControl);

            auto keyControl = std::make_shared<KeyboardSceneControl>();
            keyControl->setDevice(viewer->getKeyboardDevice());
            keyControl->setSceneManager(sceneManager);
            keyControl->setModuleDriver(driver);
            viewer->addControl(keyControl);
        }

        driver->start();
    }
}

static void
tissueCapsuleDrop()
{
    // Write log to stdout and file
    Logger::startLogger();

    // Setup a scene
    auto scene = std::make_shared<Scene>("TissueCapsuleDrop");
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setPosition(-0.0237419, 0.0368787, 0.338374);
    scene->getActiveCamera()->setViewUp(0.0, 1.0, 0.0);

    auto pbdModel  = std::make_shared<PbdModel>();
    auto pbdConfig = std::make_shared<PbdModelConfig>();
    pbdConfig->m_gravity    = Vec3d(0.0, -20.0, 0.0); // Slightly larger gravity to compensate viscosity
    pbdConfig->m_dt         = 0.005;
    pbdConfig->m_iterations = 3;
    pbdConfig->m_linearDampingCoeff  = 0.03;
    pbdConfig->m_angularDampingCoeff = 0.03;
    pbdConfig->m_doPartitioning      = false;
    pbdModel->configure(pbdConfig);

    // Setup a tissue
    std::shared_ptr<PbdObject> tissueObj = makeTissueObj("Tissue", pbdModel, 10.0, 10.0, 4, 4);
    scene->addSceneObject(tissueObj);

    // Setup capsule to drop on tissue
    auto capsuleObj = std::make_shared<PbdObject>("capsule0");
    {
        // Setup line geometry
        //auto rigidGeom = std::make_shared<Sphere>(Vec3d(0.0, 0.0, 0.0), 0.5);
        auto rigidGeom = std::make_shared<Capsule>(Vec3d(0.0, 0.1, 0.0), 0.5, 2.0);
        capsuleObj->setVisualGeometry(rigidGeom);
        capsuleObj->setCollidingGeometry(rigidGeom);
        capsuleObj->setPhysicsGeometry(rigidGeom);

        // Setup material
        capsuleObj->getVisualModel(0)->getRenderMaterial()->setColor(Color(0.9, 0.0, 0.0));
        capsuleObj->getVisualModel(0)->getRenderMaterial()->setShadingModel(RenderMaterial::ShadingModel::PBR);
        capsuleObj->getVisualModel(0)->getRenderMaterial()->setRoughness(0.5);
        capsuleObj->getVisualModel(0)->getRenderMaterial()->setMetalness(1.0);
        capsuleObj->getVisualModel(0)->getRenderMaterial()->setIsDynamicMesh(false);

        capsuleObj->setDynamicalModel(pbdModel);

        // Setup body
        capsuleObj->getPbdBody()->bodyType    = PbdBody::Type::RIGID;
        capsuleObj->getPbdBody()->initPosTest = Vec3d(-4.0, 1.0, 0.0);
        //capsuleObj->getPbdBody()->initOrientationTest = Quatd::FromTwoVectors(Vec3d(0.0, 1.0, 0.0), Vec3d(1.0, 0.0, 0.0).normalized());
        capsuleObj->getPbdBody()->uniformMassValue = 10.0; // 1.0
        /*capsuleObj->getPbdBody()->initInertiaTest = Mat3d::Identity() * 100.0;*/
        capsuleObj->getPbdBody()->initInertiaTest = Mat3d::Identity() * 100.0;
    }
    scene->addSceneObject(capsuleObj);

    auto collision = std::make_shared<PbdObjectCollision>(tissueObj, capsuleObj, "SurfaceMeshToCapsuleCD");
    //auto collision = std::make_shared<PbdObjectCollision>(tissueObj, capsuleObj, "SurfaceMeshToSphereCD");
    scene->addSceneObject(collision);

    // Light
    auto light = std::make_shared<DirectionalLight>();
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight("Light", light);

    // Run the simulation
    {
        // Setup a viewer to render
        auto viewer = std::make_shared<VTKViewer>();
        viewer->setVtkLoggerMode(VTKViewer::VTKLoggerMode::MUTE);
        viewer->setActiveScene(scene);
        viewer->setDebugAxesLength(0.01, 0.01, 0.01);

        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.001);

        // Add mouse and keyboard controls to the viewer
        {
            auto mouseControl = std::make_shared<MouseSceneControl>();
            mouseControl->setDevice(viewer->getMouseDevice());
            mouseControl->setSceneManager(sceneManager);
            viewer->addControl(mouseControl);

            auto keyControl = std::make_shared<KeyboardSceneControl>();
            keyControl->setDevice(viewer->getKeyboardDevice());
            keyControl->setSceneManager(sceneManager);
            keyControl->setModuleDriver(driver);
            viewer->addControl(keyControl);
        }

        driver->start();
    }
}

int
main()
{
    planeContactScene();

    return 0;
}