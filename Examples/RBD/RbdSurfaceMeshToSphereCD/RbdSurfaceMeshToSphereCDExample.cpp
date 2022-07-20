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
#include "imstkCollisionUtils.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkRbdConstraint.h"
#include "imstkRenderMaterial.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObject2.h"
#include "imstkRigidObjectCollision.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSphere.h"
#include "imstkSurfaceMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

std::shared_ptr<SurfaceMesh>
createBowlMesh()
{
    imstkNew<Sphere> sphere(Vec3d::Zero(), 8.0);

    std::shared_ptr<SurfaceMesh> sphereMesh = GeometryUtils::toUVSphereSurfaceMesh(sphere, 10, 10);
    auto                         sphereVerticesPtr = sphereMesh->getVertexPositions();
    auto                         sphereIndicesPtr  = sphereMesh->getCells();

    // Cut off the upper half of the sphere
    auto bowlVerticesPtr = std::make_shared<VecDataArray<double, 3>>();
    auto bowlIndicesPtr  = std::make_shared<VecDataArray<int, 3>>();

    std::unordered_map<int, int> sphereVertexToBowlVertex;
    for (int i = 0; i < sphereVerticesPtr->size(); i++)
    {
        const Vec3d sphereVertex = (*sphereVerticesPtr)[i];
        if (sphereVertex[1] < 1.0)
        {
            const int vertexId = bowlVerticesPtr->size();
            sphereVertexToBowlVertex[i] = vertexId;
            bowlVerticesPtr->push_back(sphereVertex);
        }
    }

    // Add back all triangles with all verts present
    for (int i = 0; i < sphereIndicesPtr->size(); i++)
    {
        const Vec3i& tri = (*sphereIndicesPtr)[i];
        // If all triangle vertices still present, add triangle
        if (sphereVertexToBowlVertex.count(tri[0]) != 0
            && sphereVertexToBowlVertex.count(tri[1]) != 0
            && sphereVertexToBowlVertex.count(tri[2]) != 0)
        {
            Vec3i newTri = Vec3i(
                sphereVertexToBowlVertex[tri[0]],
                sphereVertexToBowlVertex[tri[1]],
                sphereVertexToBowlVertex[tri[2]]);
            std::swap(newTri[0], newTri[1]);
            bowlIndicesPtr->push_back(newTri);
        }
    }
    auto results = std::make_shared<SurfaceMesh>();
    results->initialize(bowlVerticesPtr, bowlIndicesPtr);
    results->scale(Vec3d(1.0, 0.5, 1.0), Geometry::TransformType::ApplyToData);
    results->translate(Vec3d(0.0, 0.0, 0.0), Geometry::TransformType::ApplyToData);
    return results;
}

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
    rbdModel->getConfig()->m_maxNumIterations = 10;

    // Create the first rbd, plane floor
    imstkNew<CollidingObject> floorObj("Plane");
    {
        std::shared_ptr<SurfaceMesh> bowlMesh = createBowlMesh();

        // Create the object
        floorObj->setVisualGeometry(bowlMesh);
        floorObj->setCollidingGeometry(bowlMesh);

        auto material = std::make_shared<RenderMaterial>();
        material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
        material->setShadingModel(RenderMaterial::ShadingModel::PBR);
        material->setDiffuseColor(Color(1.0, 0.8, 0.74));
        material->setRoughness(0.5);
        material->setMetalness(0.1);
        floorObj->getVisualModel(0)->setRenderMaterial(material);

        scene->addSceneObject(floorObj);
    }

    std::array<std::shared_ptr<RigidObject2>, 6> rigidObjects;
    const int                                    rbdObjCount = static_cast<int>(rigidObjects.size());
    for (int i = 0; i < rbdObjCount; i++)
    {
        rigidObjects[i] = std::make_shared<RigidObject2>("RbdObject" + std::to_string(i));
        const double     radius = 0.8;
        imstkNew<Sphere> sphere(Vec3d::Zero(), radius);

        // Create the cube rigid object
        rigidObjects[i]->setDynamicalModel(rbdModel);
        rigidObjects[i]->setPhysicsGeometry(sphere);
        rigidObjects[i]->setCollidingGeometry(sphere);
        rigidObjects[i]->setVisualGeometry(sphere);
        rigidObjects[i]->getRigidBody()->m_mass = 1.0;
        const double t = static_cast<double>(i) / (rbdObjCount - 1);
        rigidObjects[i]->getRigidBody()->m_initPos = Vec3d((t - 0.5) * rbdObjCount * radius * 2.0, 1.0, 0.0);
        rigidObjects[i]->getRigidBody()->m_intertiaTensor = Mat3d::Identity();

        auto material = std::make_shared<RenderMaterial>();
        material->setDiffuseColor(Color::lerpRgb(Color(1.0, 0.333, 0.259), Color(0.427, 1.0, 0.58), t));
        material->setShadingModel(RenderMaterial::ShadingModel::PBR);
        material->setRoughness(0.5);
        material->setMetalness(0.5);
        rigidObjects[i]->getVisualModel(0)->setRenderMaterial(material);

        scene->addSceneObject(rigidObjects[i]);
    }

    // Collision Interaction between rigid objects
    {
        for (int i = 0; i < rbdObjCount; i++)
        {
            auto rbdInteraction = std::make_shared<RigidObjectCollision>(rigidObjects[i], floorObj, "SurfaceMeshToSphereCD");
            rbdInteraction->setFriction(0.0);
            rbdInteraction->setBaumgarteStabilization(0.0001);
            scene->addInteraction(rbdInteraction);
        }

        for (int i = 0; i < rbdObjCount; i++)
        {
            for (int j = i + 1; j < rbdObjCount; j++)
            {
                auto rbdInteraction = std::make_shared<RigidObjectCollision>(rigidObjects[i], rigidObjects[j], "SphereToSphereCD");
                rbdInteraction->setFriction(0.0);
                rbdInteraction->setBaumgarteStabilization(0.0001);
                scene->addInteraction(rbdInteraction);
            }
        }
    }

    // Camera
    scene->getActiveCamera()->setPosition(0.0252374, 2.85008, 17.0338);
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setViewUp(0.0016057, 0.999996, 0.00220191);

    // Light
    imstkNew<DirectionalLight> light;
    light->setIntensity(1.0);
    scene->addLight("light", light);

    // Run the simulation
    {
        // Setup a viewer to render in its own thread
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);
        sceneManager->pause();

        imstkNew<SimulationManager> driver;
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

                    double minDist = IMSTK_DOUBLE_MAX; // Use the closest picked sphere
                    for (int i = 0; i < rbdObjCount; i++)
                    {
                        auto sphere = std::dynamic_pointer_cast<Sphere>(rigidObjects[i]->getPhysicsGeometry());
                        Vec3d iPt;
                        if (CollisionUtils::testRayToSphere(rayStart, rayDir,
                            sphere->getCenter(), sphere->getRadius(), iPt))
                        {
                            const double dist = (iPt - rayStart).norm();
                            if (dist < minDist)
                            {
                                minDist = dist;
                                sphereSelected = i;
                                planePos       = sphere->getCenter();
                            }
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
                rbdModel->getConfig()->m_dt = sceneManager->getDt();

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
