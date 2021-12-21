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

#include "FemurObject.h"
#include "imstkCamera.h"
#include "imstkDirectionalLight.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLevelSetCH.h"
#include "imstkLevelSetModel.h"
#include "imstkMeshIO.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkRbdConstraint.h"
#include "imstkRigidBodyCH.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObject2.h"
#include "imstkRigidObjectLevelSetCollision.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkVisualModel.h"
#include "imstkVolumeRenderMaterial.h"
#include "imstkVTKViewer.h"

#ifdef iMSTK_USE_OPENHAPTICS
#include "imstkHapticDeviceClient.h"
#include "imstkHapticDeviceManager.h"
#include "imstkRigidObjectController.h"
#else
#include "imstkMouseDeviceClient.h"
#endif

using namespace imstk;

std::shared_ptr<RigidObject2>
makeRigidObj(const std::string& name)
{
    imstkNew<RigidBodyModel2> rbdModel;
    rbdModel->getConfig()->m_maxNumIterations       = 8;
    rbdModel->getConfig()->m_velocityDamping        = 1.0;
    rbdModel->getConfig()->m_angularVelocityDamping = 1.0;
    rbdModel->getConfig()->m_maxNumConstraints      = 40;

    // Create the first rbd, plane floor
    imstkNew<RigidObject2> rigidObj(name);

    {
        auto toolMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Scalpel/Scalpel_Hull_Subdivided_Shifted.stl");
        toolMesh->rotate(Vec3d(0.0, 1.0, 0.0), 3.14, Geometry::TransformType::ApplyToData);
        toolMesh->rotate(Vec3d(1.0, 0.0, 0.0), -1.57, Geometry::TransformType::ApplyToData);
        toolMesh->scale(Vec3d(0.07, 0.07, 0.07), Geometry::TransformType::ApplyToData);

        imstkNew<RenderMaterial> toolMaterial;
        toolMaterial->setDisplayMode(RenderMaterial::DisplayMode::Surface);
        toolMaterial->setShadingModel(RenderMaterial::ShadingModel::PBR);
        toolMaterial->setMetalness(0.9f);
        toolMaterial->setRoughness(0.4f);
        toolMaterial->setDiffuseColor(Color(0.7, 0.7, 0.7));

        // Create the object
        rigidObj->setVisualGeometry(toolMesh);
        rigidObj->setPhysicsGeometry(toolMesh);
        rigidObj->setCollidingGeometry(toolMesh);
        rigidObj->setDynamicalModel(rbdModel);
        rigidObj->getRigidBody()->m_mass = 10.0;
        rigidObj->getRigidBody()->m_intertiaTensor = Mat3d::Identity() * 10000.0;
        rigidObj->getRigidBody()->m_initPos = Vec3d(0.0, 1.0, 2.0);
    }
    return rigidObj;
}

///
/// \brief This example demonstrates cutting a femur bone with a tool
/// Some of the example parameters may need to be tweaked for differing
/// systems
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    imstkNew<Scene> scene("FemurCut");

    imstkNew<FemurObject> femurObj;
    scene->addSceneObject(femurObj);

    std::shared_ptr<RigidObject2> rbdObj = makeRigidObj("ToolObject");
    scene->addSceneObject(rbdObj);

    imstkNew<SceneObject> rbdGhostObj("ToolObjectGhost");
    imstkNew<SurfaceMesh> ghostMesh;
    ghostMesh->deepCopy(std::dynamic_pointer_cast<SurfaceMesh>(rbdObj->getPhysicsGeometry()));
    rbdGhostObj->setVisualGeometry(ghostMesh);
    std::shared_ptr<RenderMaterial> ghostMat = std::make_shared<RenderMaterial>(*rbdObj->getVisualModel(0)->getRenderMaterial());
    ghostMat->setOpacity(0.4);
    rbdGhostObj->getVisualModel(0)->setRenderMaterial(ghostMat);
    scene->addSceneObject(rbdGhostObj);

    imstkNew<RigidObjectLevelSetCollision> interaction(rbdObj, femurObj);
    {
        auto colHandlerA = std::dynamic_pointer_cast<RigidBodyCH>(interaction->getCollisionHandlingA());
        colHandlerA->setUseFriction(false);
        colHandlerA->setBeta(0.05); // inelastic collision

        auto colHandlerB = std::dynamic_pointer_cast<LevelSetCH>(interaction->getCollisionHandlingB());
        colHandlerB->setLevelSetVelocityScaling(0.01);
        colHandlerB->setKernel(3, 1.0);
        //colHandlerB->setLevelSetVelocityScaling(0.0); // Can't push the levelset
        colHandlerB->setUseProportionalVelocity(true);
    }
    std::shared_ptr<CollisionDetectionAlgorithm> cd = interaction->getCollisionDetection();

    scene->addInteraction(interaction);

    // Light (white)
    imstkNew<DirectionalLight> whiteLight;
    whiteLight->setDirection(Vec3d(0.0, -8.0, -5.0));
    whiteLight->setIntensity(1.0);
    scene->addLight("whiteLight", whiteLight);

    // Adjust camera
    scene->getActiveCamera()->setFocalPoint(0.25, 0.83, 1.58);
    scene->getActiveCamera()->setPosition(0.243, 1.06, 1.95);
    scene->getActiveCamera()->setViewUp(0.05, 0.86, -0.51);

    {
        imstkNew<VTKViewer> viewer;
        viewer->setVtkLoggerMode(VTKViewer::VTKLoggerMode::MUTE);
        viewer->setActiveScene(scene);

        // Add a module to run the scene
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.001); // Exactly 1000ups

#ifdef iMSTK_USE_OPENHAPTICS
        imstkNew<HapticDeviceManager> hapticManager;
        hapticManager->setSleepDelay(0.5); // Delay for 1/2ms (haptics thread is limited to max 2000hz)
        driver->addModule(hapticManager);
        std::shared_ptr<HapticDeviceClient> hapticDeviceClient = hapticManager->makeDeviceClient();

        imstkNew<RigidObjectController> controller(rbdObj, hapticDeviceClient);
        {
            controller->setLinearKd(1000.0);
            controller->setLinearKs(100000.0);
            controller->setAngularKs(300000000.0);
            controller->setAngularKd(400000.0);
            controller->setForceScaling(0.001);

            // The particular device we are using doesn't produce this quantity
            // with this flag its computed
            // in code
            controller->setComputeVelocity(true);
            controller->setComputeAngularVelocity(true);

            controller->setTranslationScaling(0.0015);
            controller->setTranslationOffset(Vec3d(0.1, 0.9, 1.6));
            controller->setSmoothingKernelSize(30);

            scene->addController(controller);
        }

        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
            {
                rbdObj->getRigidBodyModel2()->getConfig()->m_dt = sceneManager->getDt();
                femurObj->getLevelSetModel()->getConfig()->m_dt = sceneManager->getDt();

                // Also apply controller transform to ghost geometry
                ghostMesh->setTranslation(controller->getPosition());
                ghostMesh->setRotation(controller->getOrientation());
                ghostMesh->updatePostTransformData();
                ghostMesh->postModified();
            });
#else
        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
            {
                rbdObj->getRigidBodyModel2()->getConfig()->m_dt = sceneManager->getDt();
                femurObj->getLevelSetModel()->getConfig()->m_dt = sceneManager->getDt();

                const Vec2d mousePos = viewer->getMouseDevice()->getPos();
                const Vec3d worldPos = Vec3d(mousePos[0] - 0.5, mousePos[1] + 0.2, 1.575);

                const Vec3d fS = (worldPos - rbdObj->getRigidBody()->getPosition()) * 1000.0;     // Spring force
                const Vec3d fD = -rbdObj->getRigidBody()->getVelocity() * 100.0;                  // Spring damping

                (*rbdObj->getRigidBody()->m_force) += (fS + fD);

                // Also apply controller transform to ghost geometry
                ghostMesh->setTranslation(worldPos);
                ghostMesh->setRotation(Mat3d::Identity());
                ghostMesh->updatePostTransformData();
                ghostMesh->postModified();
            });
#endif

        {
            imstkNew<MouseSceneControl> mouseControl(viewer->getMouseDevice());
            mouseControl->setSceneManager(sceneManager);
            viewer->addControl(mouseControl);

            imstkNew<KeyboardSceneControl> keyControl(viewer->getKeyboardDevice());
            keyControl->setSceneManager(sceneManager);
            keyControl->setModuleDriver(driver);
            viewer->addControl(keyControl);
        }

        driver->start();
    }

    return 0;
}
