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
#include "imstkImplicitGeometryToImageData.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkPlane.h"
#include "imstkRbdConstraint.h"
#include "imstkRenderMaterial.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObject2.h"
#include "imstkRigidObjectCollision.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSphere.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief This examples demonstrates rigid body interaction between
/// primitives
///
int
main()
{
    // Write log to stdout and file
    Logger::startLogger();

    imstkNew<Scene> scene("RbdMeshMeshCollision");

    // This model is shared among interacting rigid bodies
    imstkNew<RigidBodyModel2> rbdModel;
    rbdModel->getConfig()->m_gravity = Vec3d(0.0, -9.8, 0.0);
    rbdModel->getConfig()->m_maxNumIterations = 10;

    // Create the first rbd, plane floor
    imstkNew<CollidingObject> planeObj("Plane");
    {
        imstkNew<Plane> plane(Vec3d(0.0, 0.0, 0.0), Vec3d(0.0, 1.0, 0.0));
        plane->setWidth(10.0);

        // Create the object
        planeObj->setVisualGeometry(plane);
        planeObj->setCollidingGeometry(plane);
        planeObj->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);

        scene->addSceneObject(planeObj);
    }

    // Create surface mesh cube (so we can use pointset for point->implicit collision)
    imstkNew<RigidObject2> rbdObj1("rbdObj1");
    {
        imstkNew<Sphere> sphere(Vec3d(0.0, 0.0, 0.0), 1.0);

        // Create the cube rigid object
        rbdObj1->setDynamicalModel(rbdModel);
        rbdObj1->setPhysicsGeometry(sphere);
        rbdObj1->setCollidingGeometry(sphere);
        rbdObj1->setVisualGeometry(sphere);
        rbdObj1->getRigidBody()->m_mass    = 1.0;
        rbdObj1->getRigidBody()->m_initPos = Vec3d(0.0, 8.0, 0.0);
        rbdObj1->getRigidBody()->m_intertiaTensor = Mat3d::Identity();

        scene->addSceneObject(rbdObj1);
    }

    imstkNew<RigidObject2> rbdObj2("rbdObj2");
    {
        imstkNew<Sphere> sphere(Vec3d(0.0, 0.0, 0.0), 1.0);

        // Create the cube rigid object
        rbdObj2->setDynamicalModel(rbdModel);
        rbdObj2->setPhysicsGeometry(sphere);
        rbdObj2->setCollidingGeometry(sphere);
        rbdObj2->setVisualGeometry(sphere);
        rbdObj2->getRigidBody()->m_mass    = 1.0;
        rbdObj2->getRigidBody()->m_initPos = Vec3d(0.2, 1.0, 0.0);
        rbdObj2->getRigidBody()->m_intertiaTensor = Mat3d::Identity();

        scene->addSceneObject(rbdObj2);
    }

    // Collision Interaction between plane and rbdobj1
    {
        auto rbdInteraction1 = std::make_shared<RigidObjectCollision>(rbdObj1, planeObj, "UnidirectionalPlaneToSphereCD");
        rbdInteraction1->setFriction(0.0);
        rbdInteraction1->setStiffness(0.0001);
        scene->getCollisionGraph()->addInteraction(rbdInteraction1);

        auto rbdInteraction2 = std::make_shared<RigidObjectCollision>(rbdObj2, planeObj, "UnidirectionalPlaneToSphereCD");
        rbdInteraction2->setFriction(0.0);
        rbdInteraction2->setStiffness(0.0001);
        scene->getCollisionGraph()->addInteraction(rbdInteraction2);

        auto rbdInteraction3 = std::make_shared<RigidObjectCollision>(rbdObj1, rbdObj2, "SphereToSphereCD");
        rbdInteraction3->setFriction(0.0);
        rbdInteraction3->setStiffness(0.0001);
        scene->getCollisionGraph()->addInteraction(rbdInteraction3);
    }

    // Camera
    scene->getActiveCamera()->setPosition(0.0, 40.0, 40.0);

    // Light
    imstkNew<DirectionalLight> light;
    light->setIntensity(1.0);
    scene->addLight("light", light);

    // Run the simulation
    {
        // Setup a viewer to render in its own thread
        imstkNew<VTKViewer> viewer("Viewer");
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager("Scene Manager");
        sceneManager->setActiveScene(scene);
        sceneManager->setExecutionType(Module::ExecutionType::ADAPTIVE);
        sceneManager->pause();

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.001);

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

        LOG(INFO) << "RbdObj Controls:";
        LOG(INFO) << "----------------------------------------------------------------------";
        LOG(INFO) << " | i - forward movement";
        LOG(INFO) << " | j - left movement";
        LOG(INFO) << " | l - right movement";
        LOG(INFO) << " | k - backwards movement";
        LOG(INFO) << " | u - rotate left";
        LOG(INFO) << " | o - rotate right";

        // Not perfectly thread safe movement lambda, ijkl movement instead of wasd because d is already used
        std::shared_ptr<KeyboardDeviceClient> keyDevice = viewer->getKeyboardDevice();
        connect<Event>(sceneManager, &SceneManager::postUpdate,
            [&](Event*)
        {
            // Keep cube updating at real time
            const double dt = rbdObj1->getRigidBodyModel2()->getConfig()->m_dt = sceneManager->getDt();

            Vec3d extForce  = Vec3d(0.0, 0.0, 0.0);
            Vec3d extTorque = Vec3d(0.0, 0.0, 0.0);
            // If i down, move forward
            if (keyDevice->getButton('i') == KEY_PRESS)
            {
                extForce += Vec3d(0.0, 0.0, -1500.0) * dt;
            }
            if (keyDevice->getButton('k') == KEY_PRESS)
            {
                extForce += Vec3d(0.0, 0.0, 1500.0) * dt;
            }
            if (keyDevice->getButton('j') == KEY_PRESS)
            {
                extForce += Vec3d(-1500.0, 0.0, 0.0) * dt;
            }
            if (keyDevice->getButton('l') == KEY_PRESS)
            {
                extForce += Vec3d(1500.0, 0.0, 0.0) * dt;
            }
            if (keyDevice->getButton('u') == KEY_PRESS)
            {
                extTorque += Vec3d(0.0, 1.5, 0.0) * dt;
            }
            if (keyDevice->getButton('o') == KEY_PRESS)
            {
                extTorque += Vec3d(0.0, -1.5, 0.0) * dt;
            }
            *rbdObj1->getRigidBody()->m_force  = extForce;
            *rbdObj1->getRigidBody()->m_torque = extTorque;
            });

        driver->start();
    }

    return 0;
}
