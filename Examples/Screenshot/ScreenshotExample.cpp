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
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLight.h"
#include "imstkLogger.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
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
    imstkNew<VisualObject> planeObj("VisualPlane");
    planeObj->setVisualGeometry(planeGeom);

    // Cube
    imstkNew<Cube> cubeGeom;
    cubeGeom->setWidth(0.5);
    cubeGeom->setPosition(1.0, -1.0, 0.5);
    // rotates could be replaced by cubeGeom->setOrientationAxis(1,1,1) (normalized inside)
    cubeGeom->rotate(UP_VECTOR, PI_4, Geometry::TransformType::ApplyToData);
    cubeGeom->rotate(RIGHT_VECTOR, PI_4, Geometry::TransformType::ApplyToData);
    imstkNew<VisualObject> cubeObj("VisualCube");
    cubeObj->setVisualGeometry(cubeGeom);

    // Sphere
    imstkNew<Sphere> sphereGeom;
    sphereGeom->setRadius(0.3);
    sphereGeom->setPosition(0.0, 2.0, 0.0);
    imstkNew<VisualObject> sphereObj("VisualSphere");
    sphereObj->setVisualGeometry(sphereGeom);

    // Light (white)
    imstkNew<PointLight> whiteLight("whiteLight");
    whiteLight->setIntensity(1.0);
    whiteLight->setPosition(Vec3d(5.0, 8.0, 5.0));

    // Light (red)
    imstkNew<SpotLight> colorLight("colorLight");
    colorLight->setPosition(Vec3d(4.0, -3.0, 1.0));
    colorLight->setFocalPoint(Vec3d(0.0, 0.0, 0.0));
    colorLight->setIntensity(1.0);
    colorLight->setColor(Color::Red);
    colorLight->setSpotAngle(15.0);

    // Add in scene
    scene->addSceneObject(planeObj);
    scene->addSceneObject(cubeObj);
    scene->addSceneObject(sphereObj);
    scene->addLight(whiteLight);
    scene->addLight(colorLight);

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

            connect<KeyEvent>(viewer->getKeyboardDevice(), EventType::KeyEvent,
                [&](KeyEvent* e)
            {
                if (e->m_keyPressType == KEY_PRESS)
                {
                    if (e->m_key == 'b')
                    {
                        viewer->getScreenCaptureUtility()->saveScreenShot();
                    }
                }
                });
        }

        driver->start();
    }

    return 0;
}
