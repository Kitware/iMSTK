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
#include "imstkRigidObjectCollision.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSphere.h"
#include "imstkSurfaceMesh.h"
#include "imstkSurfaceMeshFlyingEdges.h"
#include "imstkSurfaceMeshSubdivide.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"
#include "imstkCollisionDataDebugObject.h"
#include "imstkCollisionDetectionAlgorithm.h"

using namespace imstk;

///
/// \brief Creates plane geometry
/// \param cloth width
/// \param cloth height
/// \param cloth row count
/// \param cloth column count
///
static std::shared_ptr<SurfaceMesh>
makePlane(const Vec2d size,
          const Vec2i dim)
{
    imstkNew<SurfaceMesh> clothMesh;

    imstkNew<VecDataArray<double, 3>> verticesPtr(dim[0] * dim[1]);
    VecDataArray<double, 3>&          vertices = *verticesPtr.get();
    const Vec2d                       dx       = Vec2d(size[0] / static_cast<double>(dim[0] - 1), size[1] / static_cast<double>(dim[1] - 1));
    const Vec3d                       shift    = -Vec3d(size[0], 0.0, size[1]) * 0.5;
    for (int i = 0; i < dim[1]; i++)
    {
        for (int j = 0; j < dim[0]; j++)
        {
            vertices[i * dim[0] + j] = Vec3d(dx[0] * static_cast<double>(i), 0.0, dx[1] * static_cast<double>(j)) + shift;
        }
    }

    // Add connectivity data
    imstkNew<VecDataArray<int, 3>> indicesPtr;
    VecDataArray<int, 3>&          indices = *indicesPtr.get();
    for (int i = 0; i < dim[1] - 1; i++)
    {
        for (int j = 0; j < dim[0] - 1; j++)
        {
            const int index1 = i * dim[0] + j;
            const int index2 = index1 + dim[0];
            const int index3 = index1 + 1;
            const int index4 = index2 + 1;

            // Interleave [/][\]
            if (i % 2 ^ j % 2)
            {
                indices.push_back(Vec3i(index1, index3, index2));
                indices.push_back(Vec3i(index4, index2, index3));
            }
            else
            {
                indices.push_back(Vec3i(index2, index1, index4));
                indices.push_back(Vec3i(index4, index1, index3));
            }
        }
    }

    clothMesh->initialize(verticesPtr, indicesPtr);

    return clothMesh;
}

///
/// \brief This examples demonstrates rigid bodies with mesh vs mesh collision
/// with per triangle/local mesh CD
/// \todo: This example is a work in progress
///
int
main()
{
    // Write log to stdout and file
    Logger::startLogger();

    imstkNew<Scene> scene("RbdMeshMeshCollision");

    // This model is shared among interacting rigid bodies
    imstkNew<RigidBodyModel2> rbdModel;
    rbdModel->getConfig()->m_gravity = Vec3d(0.0, -2500.0, 0.0);
    rbdModel->getConfig()->m_maxNumIterations = 10;

    // Create the first rbd, plane floor
    imstkNew<CollidingObject> planeObj("Plane");
    {
        std::shared_ptr<SurfaceMesh> planeMesh = makePlane(Vec2d(25.0, 25.0), Vec2i(10, 10));

        // Create the object
        planeObj->setVisualGeometry(planeMesh);
        planeObj->setCollidingGeometry(planeMesh);
        planeObj->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);

        scene->addSceneObject(planeObj);
    }

    // Create surface mesh cube (so we can use pointset for point->implicit collision)
    imstkNew<RigidObject2> cubeObj("Cube");
    {
        imstkNew<OrientedBox>        cubeGeom(Vec3d::Zero(), Vec3d(1.5, 3.0, 1.0));
        std::shared_ptr<SurfaceMesh> cubeSurfMesh = GeometryUtils::toSurfaceMesh(cubeGeom);

        // Create the visual model
        imstkNew<VisualModel> visualModel;
        visualModel->setGeometry(cubeSurfMesh);
        imstkNew<RenderMaterial> mat;
        mat->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
        mat->setLineWidth(2.0);
        mat->setColor(Color::Orange);
        visualModel->setRenderMaterial(mat);

        // Create the cube rigid object
        cubeObj->setDynamicalModel(rbdModel);
        cubeObj->setPhysicsGeometry(cubeSurfMesh);
        cubeObj->setCollidingGeometry(cubeSurfMesh);
        cubeObj->addVisualModel(visualModel);
        cubeObj->getRigidBody()->m_mass    = 100.0;
        cubeObj->getRigidBody()->m_initPos = Vec3d(0.0, 8.0, 0.0);
        cubeObj->getRigidBody()->m_initOrientation = Quatd(Rotd(0.4, Vec3d(1.0, 0.0, 0.0)));
        cubeObj->getRigidBody()->m_intertiaTensor  = Mat3d::Identity();

        scene->addSceneObject(cubeObj);
    }

    // Collision Interaction
    auto rbdInteraction = std::make_shared<RigidObjectCollision>(cubeObj, planeObj, "SurfaceMeshToSurfaceMeshCD");
    rbdInteraction->setFriction(0.0);
    rbdInteraction->setStiffness(0.05);
    scene->getCollisionGraph()->addInteraction(rbdInteraction);

    // Camera
    scene->getActiveCamera()->setPosition(0.0, 40.0, 40.0);

    // Debug Object to display collision data
    imstkNew<CollisionDataDebugObject> debugObject;
    debugObject->setInputCD(rbdInteraction->getCollisionDetection()->getCollisionData());
    scene->addSceneObject(debugObject);

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
        driver->setDesiredDt(0.01);

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
            // If i down, move forward
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
            debugObject->debugUpdate();

            // Keep cube updating at real time
            cubeObj->getRigidBodyModel2()->getConfig()->m_dt = sceneManager->getDt();
        });

        driver->start();
    }

    return 0;
}
