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
#include "imstkSimulationManager.h"
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
    imstkNew<RigidObject> meshObj(name);

    // Load a tetrahedral mesh
    auto tetMesh = MeshIO::read<TetrahedralMesh>(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");
    tetMesh->scale(15., Geometry::TransformType::ApplyToData);
    tetMesh->translate(pos, Geometry::TransformType::ApplyToData);

    std::shared_ptr<SurfaceMesh> surfMesh = tetMesh->extractSurfaceMesh();

    // add visual model
    imstkNew<VisualModel>    renderModel(surfMesh);
    imstkNew<RenderMaterial> mat;
    mat->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    mat->setLineWidth(2.);
    mat->setColor(Color::Green);
    renderModel->setRenderMaterial(mat);
    meshObj->addVisualModel(renderModel);

    // add dynamic model
    imstkNew<RigidBodyModel>  rigidModel;
    imstkNew<RigidBodyConfig> rigidProp;
    rigidProp->m_rigidBodyType = RigidBodyType::Kinematic;
    rigidModel->configure(rigidProp);
    rigidModel->setModelGeometry(surfMesh);
    meshObj->setPhysicsGeometry(surfMesh);
    meshObj->setDynamicalModel(rigidModel);

    return meshObj;
}

std::shared_ptr<imstk::RigidObject>
makeCubeRigidObject(std::string& name, const Vec3d& pos)
{
    // Create cube object
    imstkNew<RigidObject> cubeObj(name);

    // Create cube geometry
    imstkNew<Cube> cubeGeom;
    cubeGeom->setWidth(20.0);
    cubeGeom->translate(pos);

    // Create cube VisualModel
    auto mesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/asianDragon/asianDragon.obj");
    mesh->scale(5.0, Geometry::TransformType::ApplyToData);
    imstkNew<VisualModel>    renderModel(cubeGeom.get());
    imstkNew<RenderMaterial> mat;
    mat->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    mat->setLineWidth(2.);
    mat->setColor(Color::Orange);
    renderModel->setRenderMaterial(mat);
    cubeObj->addVisualModel(renderModel);

    auto rigidMap = std::make_shared<IsometricMap>();
    rigidMap->setMaster(cubeGeom);
    rigidMap->setSlave(mesh);

    // Create cube DynamicalModel
    imstkNew<RigidBodyModel>  rigidModel;
    imstkNew<RigidBodyConfig> rigidProp;
    rigidProp->m_dynamicFriction = 0.01;
    rigidProp->m_restitution     = 0.01;
    rigidProp->m_staticFriction  = 0.005;
    rigidProp->m_rigidBodyType   = RigidBodyType::Dynamic;
    rigidModel->configure(rigidProp);
    rigidModel->setModelGeometry(cubeGeom);
    cubeObj->setDynamicalModel(rigidModel);

    return cubeObj;
}

std::shared_ptr<RigidObject>
makePlaneRigidObject()
{
    // create plane object
    imstkNew<RigidObject> planeObj("Plane");

    imstkNew<Plane> planeGeom;
    planeGeom->setWidth(400.0);

    // visual model
    imstkNew<VisualModel> renderModel(planeGeom.get());
    renderModel->setRenderMaterial(std::make_shared<RenderMaterial>());
    planeObj->addVisualModel(renderModel);

    // dynamic model
    imstkNew<RigidBodyModel>  rigidModel;
    imstkNew<RigidBodyConfig> rigidProp;
    rigidModel->configure(rigidProp);
    rigidModel->setModelGeometry(planeGeom);
    planeObj->setDynamicalModel(rigidModel);

    return planeObj;
}

std::shared_ptr<RigidObject>
makeSphereRigidObject(const Vec3d& t = Vec3d(0.0, 0.0, 0.0))
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
    imstkNew<RigidBodyConfig> rigidProp;
    rigidProp->m_rigidBodyType = RigidBodyType::Dynamic;
    rigidModel->configure(rigidProp);
    rigidModel->setModelGeometry(sphereGeom);
    sphereObj->setDynamicalModel(rigidModel);

    return sphereObj;
}

int
main()
{
    // Write log to stdout and file
    Logger::startLogger();

    imstkNew<Scene> scene("Rigid Body Dynamics");
    {
        for (int i = 0; i < 1; i++)
        {
            scene->addSceneObject(makeCubeRigidObject(std::string("cube_").append(std::to_string(i)), Vec3d(0.0, 150.0 + i * 21.0, 0.0)));
        }

        scene->addSceneObject(makePlaneRigidObject());
        scene->addSceneObject(makeSphereRigidObject(Vec3d(0.0, 200.0, 0.0)));
        scene->addSceneObject(makeMeshRigidObject(std::string("dragon"), Vec3d(0.0, 30.0, 0.0)));

        // Set Camera configuration
        scene->getActiveCamera()->setPosition(Vec3d(300.0, 300.0, 300.0));

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
        sceneManager->pause(); // Start simulation paused

        sceneManager->init();

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);

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
