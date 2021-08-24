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
#include "imstkMouseDeviceClient.h"
#include "imstkCollisionUtils.h"

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
    scene->getConfig()->taskParallelizationEnabled = false;

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

    std::shared_ptr<RigidObject2> rigidObjects[] = { nullptr, nullptr, nullptr, nullptr };
    for (int i = 0; i < 4; i++)
    {
        rigidObjects[i] = std::make_shared<RigidObject2>("RbdObject" + std::to_string(i));
        imstkNew<Sphere> sphere(Vec3d::Zero(), 0.8);

        // Create the cube rigid object
        rigidObjects[i]->setDynamicalModel(rbdModel);
        rigidObjects[i]->setPhysicsGeometry(sphere);
        rigidObjects[i]->setCollidingGeometry(sphere);
        rigidObjects[i]->setVisualGeometry(sphere);
        rigidObjects[i]->getRigidBody()->m_mass = 1.0;
        const double t = static_cast<double>(i) / 4.0;
        rigidObjects[i]->getRigidBody()->m_initPos = Vec3d(t * 8.0 - 3.0, 1.0, 0.0);
        rigidObjects[i]->getRigidBody()->m_intertiaTensor = Mat3d::Identity();
        rigidObjects[i]->getVisualModel(0)->getRenderMaterial()->setColor(Color::lerpRgb(Color::Red, Color::Blue, t));

        scene->addSceneObject(rigidObjects[i]);
    }

    // Collision Interaction between rigid objects
    {
        for (int i = 0; i < 4; i++)
        {
            auto rbdInteraction = std::make_shared<RigidObjectCollision>(rigidObjects[i], planeObj, "UnidirectionalPlaneToSphereCD");
            rbdInteraction->setFriction(0.0);
            rbdInteraction->setStiffness(0.0001);
            scene->getCollisionGraph()->addInteraction(rbdInteraction);
        }

        for (int i = 0; i < 4; i++)
        {
            for (int j = i + 1; j < 4; j++)
            {
                auto rbdInteraction = std::make_shared<RigidObjectCollision>(rigidObjects[i], rigidObjects[j], "SphereToSphereCD");
                rbdInteraction->setFriction(0.0);
                rbdInteraction->setStiffness(0.0001);
                scene->getCollisionGraph()->addInteraction(rbdInteraction);
            }
        }
    }

    // Camera
    scene->getActiveCamera()->setPosition(0.0252374, 2.85008, 17.0338);
    scene->getActiveCamera()->setFocalPoint(0.30457, 2.99155, 0.24512);
    scene->getActiveCamera()->setViewUp(0.0016057, 0.999996, 0.00220191);

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

        // The following implements pick and drag controls for a sphere, this could be more elegantly
        // implemented in a subclass of MouseControl
        LOG(INFO) << "RbdObj Controls:";
        LOG(INFO) << "----------------------------------------------------------------------";
        LOG(INFO) << " | click and drag to pick up sphere";

        // We do picking/dragging of sphere's by implementing movement on the plane whose normal is the
        // view direction and center is the sphere position when picked up
        int   sphereSelected = -1;
        Vec3d planePos;
        // Perform picking on to figure out which sphere was clicked
        connect<MouseEvent>(viewer->getMouseDevice(), &MouseDeviceClient::mouseButtonPress,
            [&](MouseEvent* e)
        {
            if (e->m_buttonId == 0)
            {
                // Get mouse position (0, 1) with origin at bot left of screen
                const Vec2d mousePos = viewer->getMouseDevice()->getPos();
                // To NDC coordinates
                const Vec3d rayDir = scene->getActiveCamera()->getEyeRayDir(
                        Vec2d(mousePos[0] * 2.0 - 1.0, mousePos[1] * 2.0 - 1.0));
                const Vec3d rayStart = scene->getActiveCamera()->getPosition();

                for (int i = 0; i < 4; i++)
                {
                    auto sphere = std::dynamic_pointer_cast<Sphere>(rigidObjects[i]->getPhysicsGeometry());
                    Vec3d iPt;
                    if (CollisionUtils::testRayToSphere(rayStart, rayDir,
                            sphere->getCenter(), sphere->getRadius(), iPt))
                    {
                        sphereSelected = i;
                        planePos       = sphere->getCenter();
                    }
                }
            }
            });
        // Unselect/drop the sphere
        connect<MouseEvent>(viewer->getMouseDevice(), &MouseDeviceClient::mouseButtonRelease,
            [&](MouseEvent* e)
        {
            if (e->m_buttonId == 0)
            {
                sphereSelected = -1;
            }
            });
        connect<Event>(sceneManager, &SceneManager::postUpdate,
            [&](Event*)
        {
            // Keep cube updating at real time
            std::shared_ptr<RigidBodyModel2> rbdModel = rigidObjects[0]->getRigidBodyModel2(); // All bodies share a model
            const double dt = rbdModel->getConfig()->m_dt = sceneManager->getDt();

            if (sphereSelected != -1)
            {
                // Get mouses current position
                const Vec2d mousePos = viewer->getMouseDevice()->getPos();
                const Vec3d rayDir   = scene->getActiveCamera()->getEyeRayDir(
                    Vec2d(mousePos[0] * 2.0 - 1.0, mousePos[1] * 2.0 - 1.0));
                const Vec3d rayStart = scene->getActiveCamera()->getPosition();

                // Exert a force to bring it to the mouse position on the plane
                auto sphere = std::dynamic_pointer_cast<Sphere>(rigidObjects[sphereSelected]->getPhysicsGeometry());
                Vec3d iPt;
                CollisionUtils::testRayToPlane(rayStart, rayDir, planePos, scene->getActiveCamera()->getForward(), iPt);
                const Vec3d fS = (iPt - sphere->getPosition()) * 100.0;                               // Spring force
                const Vec3d fD = -rigidObjects[sphereSelected]->getRigidBody()->getVelocity() * 10.0; // Spring damping
                *rigidObjects[sphereSelected]->getRigidBody()->m_force += (fS + fD);
            }
        });

        driver->start();
    }

    return 0;
}
