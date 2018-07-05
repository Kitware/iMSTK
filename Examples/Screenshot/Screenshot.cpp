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

#include "imstkSimulationManager.h"
#include "imstkCube.h"
#include "imstkSphere.h"
#include "imstkPlane.h"

using namespace imstk;

int main()
{
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto sceneTest = sdk->createNewScene("ScreenShotUtility");

    // Plane
    auto planeGeom = std::make_shared<Plane>();
    planeGeom->setWidth(10);
    auto planeObj = std::make_shared<VisualObject>("VisualPlane");
    planeObj->setVisualGeometry(planeGeom);

    // Cube
    auto cubeGeom = std::make_shared<Cube>();
    cubeGeom->setWidth(0.5);
    cubeGeom->setPosition(1.0, -1.0, 0.5);
    // rotates could be replaced by cubeGeom->setOrientationAxis(1,1,1) (normalized inside)
    cubeGeom->rotate(UP_VECTOR, PI_4, Geometry::TransformType::ApplyToData);
    cubeGeom->rotate(RIGHT_VECTOR, PI_4, Geometry::TransformType::ApplyToData);
    auto cubeObj = std::make_shared<VisualObject>("VisualCube");
    cubeObj->setVisualGeometry(cubeGeom);

    // Sphere
    auto sphereGeom = std::make_shared<Sphere>();
    sphereGeom->setRadius(0.3);
    sphereGeom->setPosition(0, 2, 0);
    auto sphereObj = std::make_shared<VisualObject>("VisualSphere");
    sphereObj->setVisualGeometry(sphereGeom);

    // Light (white)
    auto whiteLight = std::make_shared<PointLight>("whiteLight");
    whiteLight->setIntensity(100);
    whiteLight->setPosition(Vec3d(5, 8, 5));

    // Light (red)
    auto colorLight = std::make_shared<SpotLight>("colorLight");
    colorLight->setPosition(Vec3d(4, -3, 1));
    colorLight->setFocalPoint(Vec3d(0, 0, 0));
    colorLight->setIntensity(100);
    colorLight->setColor(Color::Red);
    colorLight->setSpotAngle(15);

    // Add in scene
    sceneTest->addSceneObject(planeObj);
    sceneTest->addSceneObject(cubeObj);
    sceneTest->addSceneObject(sphereObj);
    sceneTest->addLight(whiteLight);
    sceneTest->addLight(colorLight);

    // Update Camera
    auto cam1 = sceneTest->getCamera();
    cam1->setPosition(Vec3d(-5.5, 2.5, 32));
    cam1->setFocalPoint(Vec3d(1, 1, 0));

#ifndef iMSTK_USE_Vulkan
    auto viewer = sdk->getViewer();
    auto screenShotUtility
        = std::dynamic_pointer_cast<VTKScreenCaptureUtility>(viewer->getScreenCaptureUtility());
    // Set up for screen shot
    viewer->getScreenCaptureUtility()->setScreenShotPrefix("screenShot_");
    // Create a call back on key press of 'b' to take the screen shot
    viewer->setOnCharFunction('b', [&](InteractorStyle* c) -> bool
    {
        screenShotUtility->saveScreenShot();
        return false;
    });
#endif

    // Run
    sdk->setActiveScene(sceneTest);
    sdk->startSimulation(SimulationStatus::PAUSED);
    return 0;
}
