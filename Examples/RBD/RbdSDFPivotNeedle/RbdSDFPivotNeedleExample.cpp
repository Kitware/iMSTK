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

#include "imstkCamera.h"
#include "imstkCollisionGraph.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkIsometricMap.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLineMesh.h"
#include "imstkMeshIO.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkPlane.h"
#include "imstkRbdConstraint.h"
#include "imstkRenderMaterial.h"
#include "imstkRigidBodyModel2.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"
#include "NeedleInteraction.h"
#include "NeedleObject.h"

#ifdef iMSTK_USE_OPENHAPTICS
#include "imstkHapticDeviceClient.h"
#include "imstkHapticDeviceManager.h"
#include "imstkRigidObjectController.h"
#else
#include "imstkMouseDeviceClient.h"
#endif

using namespace imstk;

///
/// \brief Returns a colliding object tissue plane that uses an implicit geometry for collision
///
static std::shared_ptr<CollidingObject>
createTissueObj()
{
    imstkNew<CollidingObject> tissueObj("Tissue");

    imstkNew<Plane> tissuePlane(Vec3d(0.0, 0.0, 0.0), Vec3d(0.0, 1.0, 0.0));
    tissuePlane->setWidth(0.1);

    tissueObj->setVisualGeometry(tissuePlane);
    tissueObj->setCollidingGeometry(tissuePlane);

    auto material = std::make_shared<RenderMaterial>();
    material->setShadingModel(RenderMaterial::ShadingModel::PBR);
    material->setColor(Color::Bone);
    material->setRoughness(0.5);
    material->setMetalness(0.1);
    tissueObj->getVisualModel(0)->setRenderMaterial(material);

    return tissueObj;
}

static std::shared_ptr<NeedleObject>
createNeedleObj()
{
    imstkNew<LineMesh>                toolGeometry;
    imstkNew<VecDataArray<double, 3>> verticesPtr(2);
    (*verticesPtr)[0] = Vec3d(0.0, 0.0, 0.0);
    (*verticesPtr)[1] = Vec3d(0.0, 0.0, -0.1);
    imstkNew<VecDataArray<int, 2>> indicesPtr(1);
    (*indicesPtr)[0] = Vec2i(0, 1);
    toolGeometry->initialize(verticesPtr, indicesPtr);

    auto syringeMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Syringes/Disposable_Syringe.stl");
    syringeMesh->scale(0.0075, Geometry::TransformType::ApplyToData);
    toolGeometry->rotate(Vec3d(0.0, 1.0, 0.0), PI, Geometry::TransformType::ApplyToData);
    syringeMesh->translate(Vec3d(0.0, 0.0, 0.1), Geometry::TransformType::ApplyToData);

    imstkNew<NeedleObject> toolObj("NeedleRbdTool");
    toolObj->setVisualGeometry(syringeMesh);
    toolObj->setCollidingGeometry(toolGeometry);
    toolObj->setPhysicsGeometry(toolGeometry);
    toolObj->setPhysicsToVisualMap(std::make_shared<IsometricMap>(toolGeometry, syringeMesh));
    toolObj->getVisualModel(0)->getRenderMaterial()->setColor(Color(0.9, 0.9, 0.9));
    toolObj->getVisualModel(0)->getRenderMaterial()->setShadingModel(RenderMaterial::ShadingModel::PBR);
    toolObj->getVisualModel(0)->getRenderMaterial()->setRoughness(0.5);
    toolObj->getVisualModel(0)->getRenderMaterial()->setMetalness(1.0);
    toolObj->getVisualModel(0)->getRenderMaterial()->setOpacity(0.5);

    auto lineModel = std::make_shared<VisualModel>();
    lineModel->setGeometry(toolGeometry);
    toolObj->addVisualModel(lineModel);
    //toolObj->getVisualModel(0)->getRenderMaterial()->setLineWidth(5.0);

    std::shared_ptr<RigidBodyModel2> rbdModel = std::make_shared<RigidBodyModel2>();
    rbdModel->getConfig()->m_gravity = Vec3d::Zero();
    rbdModel->getConfig()->m_maxNumIterations       = 20;
    rbdModel->getConfig()->m_angularVelocityDamping = 0.8; // Helps with lack of 6dof
    toolObj->setDynamicalModel(rbdModel);

    toolObj->getRigidBody()->m_mass = 1.0;
    toolObj->getRigidBody()->m_intertiaTensor = Mat3d::Identity() * 1000.0;
    toolObj->getRigidBody()->m_initPos = Vec3d(0.0, 0.1, 0.0);

    return toolObj;
}

///
/// \brief This examples demonstrates rigid body with pivot constraint
///
int
main()
{
    // Write log to stdout and file
    Logger::startLogger();

    imstkNew<Scene> scene("RbdSDFNeedle");

    // Create the bone
    std::shared_ptr<CollidingObject> tissueObj = createTissueObj();
    scene->addSceneObject(tissueObj);

    // Create the needle
    std::shared_ptr<NeedleObject> needleObj = createNeedleObj();
    needleObj->setForceThreshold(50.0);
    scene->addSceneObject(needleObj);

    // Setup a debug ghost tool for virtual coupling
    auto ghostToolObj = std::make_shared<SceneObject>("ghostTool");
    {
        auto                  toolMesh = std::dynamic_pointer_cast<SurfaceMesh>(needleObj->getVisualGeometry());
        imstkNew<SurfaceMesh> toolGhostMesh;
        toolGhostMesh->initialize(
            std::make_shared<VecDataArray<double, 3>>(*toolMesh->getVertexPositions(Geometry::DataType::PreTransform)),
            std::make_shared<VecDataArray<int, 3>>(*toolMesh->getTriangleIndices()));
        ghostToolObj->setVisualGeometry(toolGhostMesh);
        ghostToolObj->getVisualModel(0)->getRenderMaterial()->setColor(Color::Orange);
        ghostToolObj->getVisualModel(0)->getRenderMaterial()->setLineWidth(5.0);
        ghostToolObj->getVisualModel(0)->getRenderMaterial()->setOpacity(0.3);
    }
    scene->addSceneObject(ghostToolObj);

    // Setup interaction between tissue and needle
    auto needleInteraction = std::make_shared<NeedleInteraction>(tissueObj, needleObj);
    scene->getCollisionGraph()->addInteraction(needleInteraction);

    // Camera
    scene->getActiveCamera()->setPosition(0.0, 0.2, 0.2);
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setViewUp(0.0, 1.0, 0.0);

    // Light
    imstkNew<DirectionalLight> light;
    light->setDirection(Vec3d(0.0, -1.0, -1.0));
    light->setIntensity(1.0);
    scene->addLight("light", light);

    // Run the simulation
    {
        // Setup a viewer to render in its own thread
        imstkNew<VTKViewer> viewer("Viewer");
        viewer->setActiveScene(scene);
        viewer->setDebugAxesLength(0.005, 0.005, 0.005);

        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager("Scene Manager");
        sceneManager->setActiveScene(scene);
        sceneManager->setExecutionType(Module::ExecutionType::ADAPTIVE);
        sceneManager->pause();

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.001);

#ifdef iMSTK_USE_OPENHAPTICS
        imstkNew<HapticDeviceManager>       hapticManager;
        std::shared_ptr<HapticDeviceClient> deviceClient = hapticManager->makeDeviceClient();
        driver->addModule(hapticManager);

        imstkNew<RigidObjectController> controller(needleObj, deviceClient);
        controller->setTranslationScaling(0.001);
        controller->setLinearKs(8000.0);
        controller->setLinearKd(200.0);
        controller->setAngularKs(1000000.0);
        controller->setAngularKd(100000.0);
        controller->setForceScaling(0.02);
        controller->setSmoothingKernelSize(5);
        controller->setUseForceSmoothening(true);
        scene->addController(controller);

        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
        {
            // Keep the tool moving in real time
            needleObj->getRigidBodyModel2()->getConfig()->m_dt = sceneManager->getDt();

            // Update the ghost debug geometry
            std::shared_ptr<Geometry> toolGhostMesh = ghostToolObj->getVisualGeometry();
            toolGhostMesh->setRotation(controller->getRotation());
            toolGhostMesh->setTranslation(controller->getPosition());
            toolGhostMesh->updatePostTransformData();
            toolGhostMesh->postModified();

            ghostToolObj->getVisualModel(0)->getRenderMaterial()->setOpacity(std::min(1.0, controller->getForce().norm() / 15.0));
            });
#else
        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
        {
            // Keep the tool moving in real time
            needleObj->getRigidBodyModel2()->getConfig()->m_dt = sceneManager->getDt();

            const Vec2d mousePos   = viewer->getMouseDevice()->getPos();
            const Vec3d desiredPos = Vec3d(mousePos[0] - 0.5, mousePos[1] - 0.5, 0.0) * 0.25;
            const Quatd desiredOrientation = Quatd(Rotd(-1.0, Vec3d(1.0, 0.0, 0.0)));

            Vec3d virutalForce;
            {
                const Vec3d fS = (desiredPos - needleObj->getRigidBody()->getPosition()) * 1000.0;   // Spring force
                const Vec3d fD = -needleObj->getRigidBody()->getVelocity() * 100.0;                  // Spring damping

                const Quatd dq       = desiredOrientation * needleObj->getRigidBody()->getOrientation().inverse();
                const Rotd angleAxes = Rotd(dq);
                const Vec3d tS       = angleAxes.axis() * angleAxes.angle() * 10000000.0;
                const Vec3d tD       = -needleObj->getRigidBody()->getAngularVelocity() * 1000.0;

                virutalForce = fS + fD;
                (*needleObj->getRigidBody()->m_force)  += virutalForce;
                (*needleObj->getRigidBody()->m_torque) += tS + tD;
            }

            // Update the ghost debug geometry
            std::shared_ptr<Geometry> toolGhostMesh = ghostToolObj->getVisualGeometry();
            toolGhostMesh->setRotation(desiredOrientation);
            toolGhostMesh->setTranslation(desiredPos);
            toolGhostMesh->updatePostTransformData();
            toolGhostMesh->postModified();

            ghostToolObj->getVisualModel(0)->getRenderMaterial()->setOpacity(std::min(1.0, virutalForce.norm() / 15.0));
            });
#endif

        // Add mouse and keyboard controls to the viewer
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
