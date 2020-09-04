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
#include "imstkCube.h"
#include "imstkGeometryUtilities.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLight.h"
#include "imstkLogger.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkPlane.h"
#include "imstkRbdConstraint.h"
#include "imstkRenderMaterial.h"
#include "imstkRigidBodyCH.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObject2.h"
#include "imstkRigidObjectCollisionPair.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;
using namespace imstk::expiremental;

///
/// \brief This example demonstrates a rigid body model. The rigid body model uses
/// an implicit model to solve all bodies together. This means they all share a model.
/// Also implements key controls in a lambda
///
int
main()
{
    // Write log to stdout and file
    Logger::startLogger();

    imstkNew<Scene> scene("Rigid Body Dynamics");
    imstkNew<RigidObject2> cubeObj("Cube");
    {
        // This model is shared among interacting rigid bodies
        imstkNew<RigidBodyModel2>       rbdModel;
        rbdModel->getConfig()->m_dt = 0.005;
        rbdModel->getConfig()->m_maxNumIterations = 10;

        // Create the first rbd, plane floor
        imstkNew<RigidObject2> planeObj("Plane");
        {
            imstkNew<Plane> planeGeom;
            planeGeom->setWidth(40.0);

            // Create the visual model
            imstkNew<VisualModel> visualModel(planeGeom.get());

            // Create the object
            planeObj->addVisualModel(visualModel);
            planeObj->setPhysicsGeometry(planeGeom);
            planeObj->setCollidingGeometry(planeGeom);
            planeObj->setDynamicalModel(rbdModel);
            planeObj->getRigidBody()->m_isStatic = true;
            planeObj->getRigidBody()->m_mass = 100.0;

            scene->addSceneObject(planeObj);
        }

        // Create surface mesh cube (so we can use pointset for point->implicit collision)
        {
            imstkNew<Cube> cubeGeom;
            cubeGeom->setWidth(4.0);
            std::shared_ptr<SurfaceMesh> surfMesh =
                GeometryUtils::toCubeSurfaceMesh(cubeGeom);

            // Create the visual model
            imstkNew<VisualModel>    visualModel(surfMesh);
            imstkNew<RenderMaterial> mat;
            mat->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
            mat->setLineWidth(2.0);
            mat->setColor(Color::Orange);
            visualModel->setRenderMaterial(mat);

            // Create the cube rigid object
            cubeObj->setDynamicalModel(rbdModel);
            cubeObj->setPhysicsGeometry(surfMesh);
            cubeObj->setCollidingGeometry(surfMesh);
            cubeObj->addVisualModel(visualModel);
            cubeObj->getRigidBody()->m_mass = 10.0;
            cubeObj->getRigidBody()->m_initPos = Vec3d(0.0, 8.0, 0.0);
            cubeObj->getRigidBody()->m_initOrientation = Quatd(Rotd(0.4, Vec3d(1.0, 0.0, 0.0)));
            cubeObj->getRigidBody()->setInertiaFromPointSet(surfMesh, 0.005);

            scene->addSceneObject(cubeObj);
        }

        auto rbdInteraction = std::make_shared<RigidObjectCollisionPair>(cubeObj, planeObj, CollisionDetection::Type::PointSetToImplicit);
        std::dynamic_pointer_cast<RigidBodyCH>(rbdInteraction->getCollisionHandlingA())->setUseFriction(false);
        scene->getCollisionGraph()->addInteraction(rbdInteraction);
        scene->getActiveCamera()->setPosition(0.0, 40.0, 40.0);

        // Light
        imstkNew<DirectionalLight> light("light");
        light->setIntensity(1.0);
        scene->addLight(light);
    }

    // Run the simulation
    {
        // Setup a viewer to render in its own thread
        imstkNew<VTKViewer> viewer("Viewer");
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager("Scene Manager");
        sceneManager->setActiveScene(scene);
        viewer->addChildThread(sceneManager); // SceneManager will start/stop with viewer

        // Add mouse and keyboard controls to the viewer
        {
            imstkNew<MouseSceneControl> mouseControl(viewer->getMouseDevice());
            mouseControl->setSceneManager(sceneManager);
            viewer->addControl(mouseControl);

            imstkNew<KeyboardSceneControl> keyControl(viewer->getKeyboardDevice());
            keyControl->setSceneManager(sceneManager);
            keyControl->setViewer(viewer);
            viewer->addControl(keyControl);
        }

        // Not perfectly thread safe movement lambda, ijkl movement instead of wasd because d is already used
        std::shared_ptr<KeyboardDeviceClient> keyDevice = viewer->getKeyboardDevice();
        const Vec3d dx = scene->getActiveCamera()->getPosition() - scene->getActiveCamera()->getFocalPoint();
        connect<Event>(sceneManager, EventType::PreUpdate, [&](Event*)
            {
                Vec3d extForce = Vec3d(0.0, 0.0, 0.0);
                Vec3d extTorque = Vec3d(0.0, 0.0, 0.0);
                // If w down, move forward
                if (keyDevice->getButton('i') == KEY_PRESS)
                {
                    extForce += Vec3d(0.0, 0.0, -100.0);
                }
                if (keyDevice->getButton('k') == KEY_PRESS)
                {
                    extForce += Vec3d(0.0, 0.0, 100.0);
                }
                if (keyDevice->getButton('j') == KEY_PRESS)
                {
                    extForce += Vec3d(-100.0, 0.0, 0.0);
                }
                if (keyDevice->getButton('l') == KEY_PRESS)
                {
                    extForce += Vec3d(100.0, 0.0, 0.0);
                }
                if (keyDevice->getButton('u') == KEY_PRESS)
                {
                    extTorque += Vec3d(0.0, 1.0, 0.0);
                }
                if (keyDevice->getButton('o') == KEY_PRESS)
                {
                    extTorque += Vec3d(0.0, -1.0, 0.0);
                }
                *cubeObj->getRigidBody()->m_force = extForce;
                *cubeObj->getRigidBody()->m_torque = extTorque;
                scene->getActiveCamera()->setFocalPoint(cubeObj->getRigidBody()->getPosition());
                scene->getActiveCamera()->setPosition(cubeObj->getRigidBody()->getPosition() + dx);
            });

        // Start viewer running, scene as paused
        sceneManager->requestStatus(ThreadStatus::Paused);
        viewer->start();
    }

    return 0;
}
