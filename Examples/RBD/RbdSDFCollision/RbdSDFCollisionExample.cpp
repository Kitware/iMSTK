/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCollider.h"
#include "imstkCompositeImplicitGeometry.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkImplicitGeometryToImageData.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkOrientedBox.h"
#include "imstkPlane.h"
#include "imstkRbdConstraint.h"
#include "imstkRenderMaterial.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObject2.h"
#include "imstkRigidObjectCollision.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkSphere.h"
#include "imstkSurfaceMesh.h"
#include "imstkSurfaceMeshFlyingEdges.h"
#include "imstkSurfaceMeshSubdivide.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief This example demonstrates rigid body collisions with an SDF.
/// It's unique in that it can handle curved surfaces well.
/// This example also implements key controls in a lambda
///
int
main()
{
    // Write log to stdout and file
    Logger::startLogger();

    auto scene   = std::make_shared<Scene>("RbdSDFCollision");
    auto cubeObj = std::make_shared<RigidObject2>("Cube");
    {
        // This model is shared among interacting rigid bodies
        auto rbdModel = std::make_shared<RigidBodyModel2>();
        rbdModel->getConfig()->m_maxNumIterations = 10;

        // Create the first rbd, plane floor
        auto planeObj = std::make_shared<SceneObject>("Plane");
        {
            // Subtract the sphere from the plane to make a crater
            auto planeGeom = std::make_shared<Plane>();
            planeGeom->setWidth(1.0);
            auto sphereGeom = std::make_shared<Sphere>();
            sphereGeom->setRadius(0.625);
            sphereGeom->setPosition(0.0, 0.4, 0.0);
            auto compGeom = std::make_shared<CompositeImplicitGeometry>();
            compGeom->addImplicitGeometry(planeGeom, CompositeImplicitGeometry::GeometryBoolType::Union);
            compGeom->addImplicitGeometry(sphereGeom, CompositeImplicitGeometry::GeometryBoolType::Difference);

            // Rasterize the SDF into an image
            ImplicitGeometryToImageData toImage;
            toImage.setInputGeometry(compGeom);
            Vec6d bounds;
            bounds[0] = -0.5;
            bounds[1] = 0.5;
            bounds[2] = -0.5;
            bounds[3] = 0.5;
            bounds[4] = -0.5;
            bounds[5] = 0.5;
            toImage.setBounds(bounds);
            toImage.setDimensions(Vec3i(80, 80, 80));
            toImage.update();

            // Extract surface
            SurfaceMeshFlyingEdges toSurfMesh;
            toSurfMesh.setInputImage(toImage.getOutputImage());
            toSurfMesh.update();
            toSurfMesh.getOutputMesh()->flipNormals();

            // Create the object
            planeObj->addComponent<VisualModel>()->setGeometry(toSurfMesh.getOutputMesh());
            planeObj->addComponent<Collider>()->setGeometry(compGeom);

            scene->addSceneObject(planeObj);
        }

        // Create surface mesh cube (so we can use pointset for point->implicit collision)
        {
            auto                         cubeGeom = std::make_shared<OrientedBox>(Vec3d::Zero(), Vec3d(0.0375, 0.075, 0.025));
            std::shared_ptr<SurfaceMesh> surfMesh = GeometryUtils::toSurfaceMesh(cubeGeom);

            SurfaceMeshSubdivide subdivide;
            subdivide.setInputMesh(surfMesh);
            subdivide.setNumberOfSubdivisions(1);
            subdivide.update();

            // Create the visual model
            auto visualModel = std::make_shared<VisualModel>();
            visualModel->setGeometry(subdivide.getOutputMesh());
            auto material = std::make_shared<RenderMaterial>();
            material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
            material->setLineWidth(2.0);
            material->setColor(Color::Orange);
            visualModel->setRenderMaterial(material);

            // Create the cube rigid object
            cubeObj->setDynamicalModel(rbdModel);
            cubeObj->setPhysicsGeometry(subdivide.getOutputMesh());
            cubeObj->addComponent<Collider>()->setGeometry(subdivide.getOutputMesh());
            cubeObj->addVisualModel(visualModel);
            cubeObj->getRigidBody()->m_mass    = 100.0;
            cubeObj->getRigidBody()->m_initPos = Vec3d(0.0, 0.2, 0.0);
            cubeObj->getRigidBody()->m_initOrientation = Quatd(Rotd(0.4, Vec3d(1.0, 0.0, 0.0)));
            cubeObj->getRigidBody()->m_intertiaTensor  = Mat3d::Identity();

            scene->addSceneObject(cubeObj);
        }

        auto rbdInteraction = std::make_shared<RigidObjectCollision>(cubeObj, planeObj, "ImplicitGeometryToPointSetCD");
        rbdInteraction->setFriction(0.0); // Don't use friction
        rbdInteraction->setBaumgarteStabilization(0.05);
        scene->addInteraction(rbdInteraction);

        // Camera
        scene->getActiveCamera()->setPosition(0.0, 1.0, 1.0);

        // Light
        auto light = std::make_shared<DirectionalLight>();
        light->setIntensity(1.0);
        scene->addLight("light", light);
    }

    // Run the simulation
    {
        // Setup a viewer to render in its own thread
        auto viewer = std::make_shared<VTKViewer>();
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene in its own thread
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->pause();

        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.001);

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
        scene->addSceneObject(mouseAndKeyControls);

        LOG(INFO) << "Cube Controls:";
        LOG(INFO) << "----------------------------------------------------------------------";
        LOG(INFO) << " | i - forward movement";
        LOG(INFO) << " | j - left movement";
        LOG(INFO) << " | l - right movement";
        LOG(INFO) << " | k - backwards movement";
        LOG(INFO) << " | u - rotate left";
        LOG(INFO) << " | o - rotate right";

        std::shared_ptr<KeyboardDeviceClient> keyDevice = viewer->getKeyboardDevice();

        const Vec3d  dx    = scene->getActiveCamera()->getPosition() - scene->getActiveCamera()->getFocalPoint();
        const double speed = 200.0;
        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
            {
                Vec3d extForce  = Vec3d(0.0, 0.0, 0.0);
                Vec3d extTorque = Vec3d(0.0, 0.0, 0.0);
                // If w down, move forward
                if (keyDevice->getButton('i') == KEY_PRESS)
                {
                    extForce += Vec3d(0.0, 0.0, -1.0) * speed;
                }
                if (keyDevice->getButton('k') == KEY_PRESS)
                {
                    extForce += Vec3d(0.0, 0.0, 1.0) * speed;
                }
                if (keyDevice->getButton('j') == KEY_PRESS)
                {
                    extForce += Vec3d(-1.0, 0.0, 0.0) * speed;
                }
                if (keyDevice->getButton('l') == KEY_PRESS)
                {
                    extForce += Vec3d(1.0, 0.0, 0.0) * speed;
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
