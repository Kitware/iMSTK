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
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkDirectionalLight.h"
#include "imstkPointLight.h"
#include "imstkSpotLight.h"
#include "imstkLogger.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkOrientedBox.h"
#include "imstkPlane.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneObject.h"
#include "imstkSimulationManager.h"
#include "imstkSphere.h"
#include "imstkViewer.h"
#include "imstkVTKScreenCaptureUtility.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief This example demonstrates how to capture the screenshot
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    imstkNew<Scene> scene("ScreenShotUtility");

    // Plane
    imstkNew<Plane> planeGeom;
    planeGeom->setWidth(10.0);
    imstkNew<SceneObject> planeObj("VisualPlane");
    planeObj->setVisualGeometry(planeGeom);

    // Cube
    imstkNew<OrientedBox> cubeGeom(Vec3d(1.0, -1.0, 0.5), Vec3d(0.25, 0.25, 0.25));
    cubeGeom->rotate(UP_VECTOR, PI_4, Geometry::TransformType::ApplyToData);
    cubeGeom->rotate(RIGHT_VECTOR, PI_4, Geometry::TransformType::ApplyToData);
    imstkNew<SceneObject> cubeObj("VisualCube");
    cubeObj->setVisualGeometry(cubeGeom);

    // Sphere
    imstkNew<Sphere> sphereGeom;
    sphereGeom->setRadius(0.3);
    sphereGeom->setPosition(0.0, 2.0, 0.0);
    imstkNew<SceneObject> sphereObj("VisualSphere");
    sphereObj->setVisualGeometry(sphereGeom);

    // Light (white)
    imstkNew<PointLight> whiteLight;
    whiteLight->setIntensity(1.0);
    whiteLight->setPosition(Vec3d(5.0, 8.0, 5.0));

    // Light (red)
    imstkNew<SpotLight> colorLight;
    colorLight->setPosition(Vec3d(4.0, -3.0, 1.0));
    colorLight->setFocalPoint(Vec3d(0.0, 0.0, 0.0));
    colorLight->setIntensity(1.0);
    colorLight->setColor(Color::Red);
    colorLight->setSpotAngle(15.0);

    // Add in scene
    scene->addSceneObject(planeObj);
    scene->addSceneObject(cubeObj);
    scene->addSceneObject(sphereObj);
    scene->addLight("whitelight", whiteLight);
    scene->addLight("colorlight", colorLight);

    // Update Camera
    scene->getActiveCamera()->setPosition(Vec3d(-5.5, 2.5, 32.0));
    scene->getActiveCamera()->setFocalPoint(Vec3d(1.0, 1.0, 0.0));

    LOG(INFO) << "PRESS 'b' for taking screenshots";

    // Run the simulation
    {
        // Setup a viewer to render in its own thread
        imstkNew<VTKViewer> viewer("Viewer");
        viewer->setActiveScene(scene);
        viewer->getScreenCaptureUtility()->setScreenShotPrefix("screenShot_");

        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager("Scene Manager");
        sceneManager->setActiveScene(scene);

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

            connect<KeyEvent>(viewer->getKeyboardDevice(), &KeyboardDeviceClient::keyPress,
                [&](KeyEvent* e)
            {
                if (e->m_key == 'b')
                {
                    viewer->getScreenCaptureUtility()->saveScreenShot();
                }
                });
        }

        driver->start();
    }

    return 0;
}
