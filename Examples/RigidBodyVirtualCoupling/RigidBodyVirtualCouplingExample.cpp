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
#include "imstkCube.h"
#include "imstkHapticDeviceClient.h"
#include "imstkHapticDeviceManager.h"
#include "imstkIsometricMap.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLight.h"
#include "imstkLogger.h"
#include "imstkMeshIO.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkPlane.h"
#include "imstkRenderMaterial.h"
#include "imstkRigidBodyModel.h"
#include "imstkRigidObject.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneObjectController.h"
#include "imstkSphere.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

std::shared_ptr<imstk::RigidObject>
makeMeshRigidObject(const std::string& name, const Vec3d& pos)
{
    // create cube object
    imstkNew<RigidObject> meshObj("name");

    // Load a tetrahedral mesh
    auto tetMesh = MeshIO::read<TetrahedralMesh>(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");
    if (!tetMesh)
    {
        LOG(FATAL) << "Could not read mesh from file.";
    }

    // Extract the surface mesh
    imstkNew<SurfaceMesh> surfMesh;
    tetMesh->scale(15.0, Geometry::TransformType::ApplyToData);
    tetMesh->translate(pos, Geometry::TransformType::ApplyToData);
    tetMesh->extractSurfaceMesh(surfMesh, true);

    // Add visual model
    imstkNew<VisualModel>    renderModel(surfMesh.get());
    imstkNew<RenderMaterial> mat;
    mat->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    mat->setLineWidth(2.0);
    mat->setColor(Color::Green);
    renderModel->setRenderMaterial(mat);
    meshObj->addVisualModel(renderModel);

    // add dynamic model
    imstkNew<RigidBodyModel>  rigidModel;
    imstkNew<RigidBodyConfig> rigidConfig;
    rigidConfig->m_rigidBodyType = RigidBodyType::Kinematic;
    rigidModel->configure(rigidConfig);
    rigidModel->setModelGeometry(surfMesh);
    meshObj->setPhysicsGeometry(surfMesh);
    meshObj->setDynamicalModel(rigidModel);

    return meshObj;
}

std::shared_ptr<imstk::RigidObject>
makeCubeRigidObject(const std::string& name, const Vec3d& pos, const bool isStatic)
{
    // create cube object
    imstkNew<RigidObject> cubeObj(name);

    // Create Cube object
    imstkNew<Cube> cubeGeom(pos, 20.0);

    // cube visual model
    auto surfMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/asianDragon/asianDragon.obj");
    surfMesh->scale(5., Geometry::TransformType::ApplyToData);
    imstkNew<VisualModel>    renderModel(cubeGeom.get());
    imstkNew<RenderMaterial> mat;
    mat->setDisplayMode(RenderMaterial::DisplayMode::Surface);
    mat->setLineWidth(2.0);
    mat->setColor(Color::Orange);
    renderModel->setRenderMaterial(mat);
    cubeObj->addVisualModel(renderModel);

    imstkNew<IsometricMap> rigidMap;
    rigidMap->setMaster(cubeGeom);
    rigidMap->setSlave(surfMesh);

    // cube dynamic model
    imstkNew<RigidBodyModel>  rigidModel;
    imstkNew<RigidBodyConfig> rigidConfig;
    rigidConfig->m_rigidBodyType = RigidBodyType::Dynamic;
    rigidModel->configure(rigidConfig);
    rigidModel->setModelGeometry(cubeGeom);
    cubeObj->setDynamicalModel(rigidModel);

    cubeObj->setPhysicsToVisualMap(rigidMap);

    return cubeObj;
}

std::shared_ptr<RigidObject>
makePlaneRigidObject(const double width)
{
    // create plane object
    imstkNew<RigidObject> planeObj("Plane");

    imstkNew<Plane> planeGeom;
    planeGeom->setWidth(width);

    // visual model
    imstkNew<VisualModel> renderModel(planeGeom.get());
    renderModel->setRenderMaterial(std::make_shared<RenderMaterial>());
    planeObj->addVisualModel(renderModel);

    // dynamic model
    imstkNew<RigidBodyModel>  rigidModel;
    imstkNew<RigidBodyConfig> rigidConfig;
    rigidConfig->m_rigidBodyType = RigidBodyType::Static;
    rigidModel->configure(rigidConfig);
    rigidModel->setModelGeometry(planeGeom);
    planeObj->setDynamicalModel(rigidModel);

    return planeObj;
}

std::shared_ptr<RigidObject>
makeSphereRigidObject(const Vec3d& t)
{
    // create cube object
    imstkNew<RigidObject> sphereObj("Sphere");

    // Create Cube object
    imstkNew<Sphere> sphereGeom;
    sphereGeom->setRadius(10.0);
    sphereGeom->translate(t);

    // cube visual model
    imstkNew<VisualModel> renderModel(sphereGeom.get());
    renderModel->setRenderMaterial(std::make_shared<RenderMaterial>());
    sphereObj->addVisualModel(renderModel);

    // cube dynamic model
    imstkNew<RigidBodyModel>  rigidModel;
    imstkNew<RigidBodyConfig> rigidConfig;
    rigidConfig->m_rigidBodyType = RigidBodyType::Dynamic;
    rigidModel->configure(rigidConfig);
    rigidModel->setModelGeometry(sphereGeom);
    sphereObj->setDynamicalModel(rigidModel);

    return sphereObj;
}

int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Create Scene
    imstkNew<Scene> scene("ControlRB");

    std::shared_ptr<RigidObject> cubeObj = makeCubeRigidObject("cube", Vec3d(0.0, 0.0, 0.0), false);
    scene->addSceneObject(cubeObj);

    std::shared_ptr<RigidObject> planeObj = makePlaneRigidObject(400.0);
    scene->addSceneObject(planeObj);

    //makeSphereRigidObject(Vec3d(0.0, 200.0, 0..0));

    std::shared_ptr<RigidObject> meshObj = makeMeshRigidObject("dragon", Vec3d(0.0, 30.0, 0.0));
    scene->addSceneObject(meshObj);

    //-------------------------------------------------------------

    // Device Server
    imstkNew<HapticDeviceManager>       server;
    const std::string                   deviceName = "";
    std::shared_ptr<HapticDeviceClient> client     = server->makeDeviceClient(deviceName);

    // Create a virtual coupling object
    imstkNew<Sphere>         visualGeom(Vec3d(0.0, 0.0, 0.0), 5.0);
    imstkNew<VisualObject>   obj("virtualCouplingObject");
    imstkNew<RenderMaterial> material;
    imstkNew<VisualModel>    visualModel(visualGeom.get());
    visualModel->setRenderMaterial(material);
    obj->addVisualModel(visualModel);
    scene->addSceneObject(obj);

    // Device tracker
    imstkNew<SceneObjectController> controller(obj, client);
    scene->addController(controller);

    //-----------------------------------------------------------------
    std::shared_ptr<RigidBodyModel> rbdModel = cubeObj->getRigidBodyModel();

    if (!rbdModel.get())
    {
        std::shared_ptr<RigidObject> cubeObj2 = makeCubeRigidObject("cube", Vec3d(0.0, 40.0, 0.0), false);
        scene->addSceneObject(cubeObj2);
        // throw error
    }

    // Set Camera configuration
    scene->getActiveCamera()->setPosition(Vec3d(300.0, 300.0, 300.0));

    // Light
    imstkNew<DirectionalLight> light("light");
    light->setIntensity(1.0);
    scene->addLight(light);

    // Run
    //Run the simulation
    {
        // Setup a viewer to render in its own thread
        imstkNew<VTKViewer> viewer("Viewer 1");
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager("Scene Manager 1");
        sceneManager->setActiveScene(scene);
        viewer->addChildThread(sceneManager); // SceneManager will start/stop with viewer

        viewer->addChildThread(server);

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

        Vec3d prevCubePos = rbdModel->getModelGeometry()->getTranslation();
        connect<Event>(sceneManager, EventType::PostUpdate,
            [&](Event*)
        {
            const auto devPos = controller->getPosition();
            const auto devQ   = controller->getRotation();
            rbdModel->getModelGeometry()->rotate(devQ);
            auto cubeGeo = std::dynamic_pointer_cast<Cube>(cubeObj->getPhysicsGeometry());
            const Vec3d cubePos      = rbdModel->getModelGeometry()->getTranslation();
            const Vec3d cubeVelocity = (cubePos - prevCubePos) / 2.0;
            const Vec3d damp  = -1000000 * cubeVelocity;
            const Vec3d force = -1000 * (cubePos - devPos) + damp;
            rbdModel->addForce(force, Vec3d(0.0, 0.0, 0.0));
            prevCubePos = cubePos;
            });

        // Start viewer running, scene as paused
        sceneManager->requestStatus(ThreadStatus::Paused);
        viewer->start();
    }

    return 0;
}