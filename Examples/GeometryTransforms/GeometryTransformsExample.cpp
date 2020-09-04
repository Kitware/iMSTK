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

#include "imstkAPIUtilities.h"
#include "imstkCamera.h"
#include "imstkCube.h"
#include "imstkCylinder.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLight.h"
#include "imstkLogger.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkPlane.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneObject.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief This example demonstrates the geometry transforms in imstk
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    imstkNew<Scene> scene("GeometryTransforms");

    auto sceneObj = apiutils::createAndAddVisualSceneObject(scene, iMSTK_DATA_ROOT "/asianDragon/asianDragon.obj", "Dragon");

    CHECK(sceneObj != nullptr) << "ERROR: Unable to create scene object";

    auto surfaceMesh = sceneObj->getVisualGeometry();
    surfaceMesh->scale(5.0, Geometry::TransformType::ConcatenateToTransform);

    //  Plane
    imstkNew<Plane> planeGeom;
    planeGeom->scale(80.0, Geometry::TransformType::ConcatenateToTransform);
    planeGeom->translate(0.0, -20.0, 0.0, Geometry::TransformType::ConcatenateToTransform);
    planeGeom->rotate(Vec3d(0.0, 1.0, 0.0), PI_4, Geometry::TransformType::ConcatenateToTransform);

    imstkNew<RenderMaterial> planeMaterial;
    planeMaterial->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    planeMaterial->setPointSize(6.);
    planeMaterial->setLineWidth(4.);
    imstkNew<VisualModel> planeVisualModel(planeGeom.get());
    planeVisualModel->setRenderMaterial(planeMaterial);

    imstkNew<VisualObject> planeObj("Plane");
    //planeObj->setVisualGeometry(planeGeom);
    planeObj->addVisualModel(planeVisualModel);
    scene->addSceneObject(planeObj);

    //  Cube
    imstkNew<Cube> cubeGeom;
    cubeGeom->setWidth(20.0);
    cubeGeom->scale(0.5, Geometry::TransformType::ConcatenateToTransform);
    cubeGeom->rotate(Vec3d(1.0, 1.0, 0.0), PI_4, Geometry::TransformType::ApplyToData);

    auto materialCube = std::make_shared<RenderMaterial>();
    imstkNew<RenderMaterial> cubeMaterial;
    cubeMaterial->setColor(imstk::Color::Red);
    cubeMaterial->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    cubeMaterial->setPointSize(6.);
    cubeMaterial->setLineWidth(4.);
    imstkNew<VisualModel> cubeVisualModel(cubeGeom.get());
    cubeVisualModel->setRenderMaterial(cubeMaterial);

    imstkNew<VisualObject> cubeObj("Cube");
    cubeObj->addVisualModel(cubeVisualModel);
    scene->addSceneObject(cubeObj);

    //  Cylinder
    imstkNew<Cylinder> cylinderGeom;
    cylinderGeom->setRadius(4.0);
    cylinderGeom->setLength(8.0);
    cylinderGeom->scale(0.4, Geometry::TransformType::ConcatenateToTransform);
    cylinderGeom->rotate(Vec3d(1.0, 1.0, 0), PI_2, Geometry::TransformType::ApplyToData);

    imstkNew<RenderMaterial> cylMaterial;
    cylMaterial->setColor(imstk::Color::Red);
    cylMaterial->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    cylMaterial->setPointSize(6.0);
    cylMaterial->setLineWidth(4.0);
    imstkNew<VisualModel> cylVisualModel(cylinderGeom.get());
    cylVisualModel->setRenderMaterial(materialCube);

    imstkNew<VisualObject> cylObj("Cylinder");
    cylObj->addVisualModel(cylVisualModel);
    scene->addSceneObject(cylObj);

    // Setup function to rotate the dragon every frame
    auto rotateFunc =
        [&surfaceMesh](Event*)
        {
            surfaceMesh->rotate(Vec3d(1.0, 0.0, 0.0), PI * 0.0001, Geometry::TransformType::ApplyToData);
        };

    // Set Camera configuration
    scene->getActiveCamera()->setPosition(Vec3d(0.0, 30.0, 30.0));

    // Light
    imstkNew<DirectionalLight> light("light");
    light->setDirection(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight(light);

    // Run the simulation
    {
        // Setup a viewer to render in its own thread
        imstkNew<VTKViewer> viewer("Viewer");
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager("Scene Manager");
        sceneManager->setActiveScene(scene);
        viewer->addChildThread(sceneManager); // SceneManager will start/stop with viewer
        connect<Event>(sceneManager, EventType::PostUpdate, rotateFunc);

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

        viewer->start();
    }

    return 0;
}
