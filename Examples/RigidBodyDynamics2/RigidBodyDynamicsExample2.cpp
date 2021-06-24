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
#include "imstkCompositeImplicitGeometry.h"
#include "imstkGeometryUtilities.h"
#include "imstkImplicitGeometryToImageData.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkDirectionalLight.h"
#include "imstkLogger.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkOrientedBox.h"
#include "imstkPlane.h"
#include "imstkRbdConstraint.h"
#include "imstkRenderMaterial.h"
#include "imstkRigidBodyCH.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObject2.h"
#include "imstkRigidObjectCollidingCollisionPair.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSphere.h"
#include "imstkSurfaceMesh.h"
#include "imstkSurfaceMeshFlyingEdges.h"
#include "imstkSurfaceMeshSubdivide.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

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

    imstkNew<Scene>        scene("Rigid Body Dynamics");
    imstkNew<RigidObject2> cubeObj("Cube");
    {
        // This model is shared among interacting rigid bodies
        imstkNew<RigidBodyModel2> rbdModel;
        rbdModel->getConfig()->m_gravity = Vec3d(0.0, -2500.0, 0.0);
        rbdModel->getConfig()->m_maxNumIterations = 10;

        // Create the first rbd, plane floor
        imstkNew<CollidingObject> planeObj("Plane");
        {
            // Subtract the sphere from the plane to make a crater
            imstkNew<Plane> planeGeom;
            planeGeom->setWidth(40.0);
            imstkNew<Sphere> sphereGeom;
            sphereGeom->setRadius(25.0);
            sphereGeom->setPosition(0.0, 10.0, 0.0);
            imstkNew<CompositeImplicitGeometry> compGeom;
            compGeom->addImplicitGeometry(planeGeom, CompositeImplicitGeometry::GeometryBoolType::Union);
            compGeom->addImplicitGeometry(sphereGeom, CompositeImplicitGeometry::GeometryBoolType::Difference);

            // Rasterize the SDF into an image
            imstkNew<ImplicitGeometryToImageData> toImage;
            toImage->setInputGeometry(compGeom);
            Vec6d bounds;
            bounds[0] = -20.0;
            bounds[1] = 20.0;
            bounds[2] = -20.0;
            bounds[3] = 20.0;
            bounds[4] = -20.0;
            bounds[5] = 20.0;
            toImage->setBounds(bounds);
            toImage->setDimensions(Vec3i(80, 80, 80));
            toImage->update();

            // Extract surface
            imstkNew<SurfaceMeshFlyingEdges> toSurfMesh;
            toSurfMesh->setInputImage(toImage->getOutputImage());
            toSurfMesh->update();
            toSurfMesh->getOutputMesh()->flipNormals();

            // Create the visual model
            imstkNew<VisualModel> visualModel(toSurfMesh->getOutputMesh());

            // Create the object
            planeObj->addVisualModel(visualModel);
            planeObj->setCollidingGeometry(compGeom);
            //planeObj->getRigidBody()->m_isStatic = true;
            //planeObj->getRigidBody()->m_mass     = 100.0;

            scene->addSceneObject(planeObj);
        }

        // Create surface mesh cube (so we can use pointset for point->implicit collision)
        {
            imstkNew<OrientedBox>        cubeGeom(Vec3d::Zero(), Vec3d(1.5, 3.0, 1.0));
            std::shared_ptr<SurfaceMesh> surfMesh = GeometryUtils::toSurfaceMesh(cubeGeom);

            imstkNew<SurfaceMeshSubdivide> subdivide;
            subdivide->setInputMesh(surfMesh);
            subdivide->setNumberOfSubdivisions(1);
            subdivide->update();

            // Create the visual model
            imstkNew<VisualModel>    visualModel(subdivide->getOutputMesh());
            imstkNew<RenderMaterial> mat;
            mat->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
            mat->setLineWidth(2.0);
            mat->setColor(Color::Orange);
            visualModel->setRenderMaterial(mat);

            // Create the cube rigid object
            cubeObj->setDynamicalModel(rbdModel);
            cubeObj->setPhysicsGeometry(subdivide->getOutputMesh());
            cubeObj->setCollidingGeometry(subdivide->getOutputMesh());
            cubeObj->addVisualModel(visualModel);
            cubeObj->getRigidBody()->m_mass    = 100.0;
            cubeObj->getRigidBody()->m_initPos = Vec3d(0.0, 8.0, 0.0);
            cubeObj->getRigidBody()->m_initOrientation = Quatd(Rotd(0.4, Vec3d(1.0, 0.0, 0.0)));
            cubeObj->getRigidBody()->m_intertiaTensor  = Mat3d::Identity();

            scene->addSceneObject(cubeObj);
        }

        auto                         rbdInteraction = std::make_shared<RigidObjectCollidingCollisionPair>(cubeObj, planeObj, CollisionDetection::Type::PointSetToImplicit);
        std::shared_ptr<RigidBodyCH> ch = std::dynamic_pointer_cast<RigidBodyCH>(rbdInteraction->getCollisionHandlingA());
        ch->setUseFriction(false);
        ch->setStiffness(0.05);
        scene->getCollisionGraph()->addInteraction(rbdInteraction);
        scene->getActiveCamera()->setPosition(0.0, 40.0, 40.0);

        // Light
        imstkNew<DirectionalLight> light;
        light->setIntensity(1.0);
        scene->addLight("light", light);
    }

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

        LOG(INFO) << "Cube Controls:";
        LOG(INFO) << "----------------------------------------------------------------------";
        LOG(INFO) << " | i - forward movement";
        LOG(INFO) << " | j - left movement";
        LOG(INFO) << " | l - right movement";
        LOG(INFO) << " | k - backwards movement";
        LOG(INFO) << " | u - rotate left";
        LOG(INFO) << " | o - rotate right";

        // Not perfectly thread safe movement lambda, ijkl movement instead of wasd because d is already used
        std::shared_ptr<KeyboardDeviceClient> keyDevice = viewer->getKeyboardDevice();
        const Vec3d                           dx = scene->getActiveCamera()->getPosition() - scene->getActiveCamera()->getFocalPoint();
        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
        {
            Vec3d extForce  = Vec3d(0.0, 0.0, 0.0);
            Vec3d extTorque = Vec3d(0.0, 0.0, 0.0);
            // If w down, move forward
            if (keyDevice->getButton('i') == KEY_PRESS)
            {
                extForce += Vec3d(0.0, 0.0, -900.0);
            }
            if (keyDevice->getButton('k') == KEY_PRESS)
            {
                extForce += Vec3d(0.0, 0.0, 900.0);
            }
            if (keyDevice->getButton('j') == KEY_PRESS)
            {
                extForce += Vec3d(-900.0, 0.0, 0.0);
            }
            if (keyDevice->getButton('l') == KEY_PRESS)
            {
                extForce += Vec3d(900.0, 0.0, 0.0);
            }
            if (keyDevice->getButton('u') == KEY_PRESS)
            {
                extTorque += Vec3d(0.0, 1.5, 0.0);
            }
            if (keyDevice->getButton('o') == KEY_PRESS)
            {
                extTorque += Vec3d(0.0, -1.5, 0.0);
            }
            *cubeObj->getRigidBody()->m_force  = extForce;
            *cubeObj->getRigidBody()->m_torque = extTorque;
            scene->getActiveCamera()->setFocalPoint(cubeObj->getRigidBody()->getPosition());
            scene->getActiveCamera()->setPosition(cubeObj->getRigidBody()->getPosition() + dx);
        });
        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
        {
            cubeObj->getRigidBodyModel2()->getConfig()->m_dt = sceneManager->getDt();
        });

        driver->start();
    }

    return 0;
}
