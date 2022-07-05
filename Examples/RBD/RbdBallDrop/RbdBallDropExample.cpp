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
#include "imstkPlane.h"
#include "imstkUnidirectionalPlaneToSphereCD.h"

using namespace imstk;

namespace
{
} // namespace

///
/// \brief This examples demonstrates rigid body interaction between
/// primitives
///
int
main()
{
    // Write log to stdout and file
    Logger::startLogger();

    auto scene = std::make_shared<Scene>("RbdMeshMeshCollision");

    // This model is shared among interacting rigid bodies
    // In most cases instances of iMSTK object should be created
    // on the heap and stored in std::shared_ptr
    auto rbdModel = std::make_shared<RigidBodyModel2>();

    // iMSTK coordinates are right handed with Y pointing up
    // While we have defined types for Vectors and Matrices
    // these are all Eigen https://eigen.tuxfamily.org/index.php
    // types and all of Eigen is available in iMSTK
    rbdModel->getConfig()->m_gravity = Vec3d(0.0, -9.8, 0.0);
    rbdModel->getConfig()->m_maxNumIterations = 10;

    // iMSTK knows about various kinds of geometry, amongst others
    // analytical shapes and meshes, here a analytical sphere is used
    auto sphere = std::make_shared<Sphere>(Vec3d(0, 0, 0), 0.5);

    // Create a SceneObject that is a RigidObject
    // Then we set up the object with all the data necessary
    auto sphereObject = std::make_shared<RigidObject2>("Sphere");

    // Give the object something to visualize, this can be shortcut by using
    // addVisualGeometry()
    auto sphereVisualModel = std::make_shared<VisualModel>();
    sphereVisualModel->setGeometry(sphere);
    sphereObject->addVisualModel(sphereVisualModel);

    // Customize the look using a material
    auto sphereMaterial = std::make_shared<RenderMaterial>();
    sphereMaterial->setDiffuseColor(Color(1.0, 0.333, 0.259));
    sphereMaterial->setShadingModel(RenderMaterial::ShadingModel::PBR);
    sphereMaterial->setRoughness(0.5);
    sphereMaterial->setMetalness(0.5);
    sphereVisualModel->setRenderMaterial(sphereMaterial);

    // This is the geometry used for collision detection
    sphereObject->setCollidingGeometry(sphere);

    // Forces and physical effects are applied to the physics geometry
    sphereObject->setPhysicsGeometry(sphere);

    sphereObject->setDynamicalModel(rbdModel);
    sphereObject->getRigidBody()->m_mass = 1.0;

    sphereObject->getRigidBody()->m_initPos = Vec3d(0.0, 3.0, 0.0);
    sphereObject->getRigidBody()->m_intertiaTensor = Mat3d::Identity();

    // Finally add the object to the scene
    scene->addSceneObject(sphereObject);

    // Create a plane at the origin with a normal along the Y-Axis
    auto plane = std::make_shared<Plane>(Vec3d::Zero(), Vec3d::UnitY());

    // This "width" is just used visually, this is an analytical plane
    // that is infinite
    plane->setWidth(10.0);

    // A CollidingObject is static and will not react to forces
    auto planeObject = std::make_shared<CollidingObject>("Plane");
    planeObject->setVisualGeometry(plane);
    planeObject->setCollidingGeometry(plane);
    scene->addSceneObject(planeObject);

    // To affect collision detection and response the appropriate interaction has to be instantiated
    // and added to the scene, the type of the interaction is dependent on the interacting model types
    // in this case Rigid and Colliding (Static) object
    // the type of the collision detection is dependent on the kinds of geometry involved
    // Add collisions for all the rigid bodies with the bowl
    scene->addInteraction(
        std::make_shared<RigidObjectCollision>(sphereObject, planeObject, UnidirectionalPlaneToSphereCD::getStaticTypeName()));

    // Camera
    scene->getActiveCamera()->setPosition(0, 3, 20);
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setViewUp(0, 1, 0);

    // Light
    imstkNew<DirectionalLight> light;
    light->setIntensity(1.0);
    scene->addLight("light", light);

    // Setup a viewer to render in its own thread
    auto viewer = std::make_shared<VTKViewer>();
    viewer->setActiveScene(scene);

    // Setup a scene manager to advance the scene in its own thread
    auto sceneManager = std::make_shared<SceneManager>();
    sceneManager->setActiveScene(scene);
    sceneManager->pause();

    // The simulation manager is responsible to direct multiple modules
    auto simulationManager = std::make_shared<SimulationManager>();
    simulationManager->addModule(viewer);
    simulationManager->addModule(sceneManager);
    simulationManager->setDesiredDt(0.001);

    auto mouseControl = std::make_shared<MouseSceneControl>();
    mouseControl->setDevice(viewer->getMouseDevice());
    mouseControl->setSceneManager(sceneManager);
    viewer->addControl(mouseControl);

    auto keyControl = std::make_shared<KeyboardSceneControl>();
    keyControl->setDevice(viewer->getKeyboardDevice());
    keyControl->setSceneManager(sceneManager);
    keyControl->setModuleDriver(simulationManager);
    viewer->addControl(keyControl);

    // iMSTK utilizes an event system this is one of many ways to connect to this system
    // postUpdateFunction will be called every time the SceneManager posts a
    // postUdate event, in this case a lambda is used to capture the scene by
    // value as we don't have an object scope to use use, depending on your needs a free function
    // or a member function can be used as callbacks for events.
    connect<Event>(sceneManager, &SceneManager::postUpdate, [scene](Event*) {
            // Remove comment to print out the elapsed time every frame
            // LOG(INFO) << "Elapsed: " << scene->getSceneTime();
        });

    // By default the scene will start "paused" press space to start the simulation
    // look at the console for more instructions about available key strokes
    simulationManager->start();

    return 0;
}
