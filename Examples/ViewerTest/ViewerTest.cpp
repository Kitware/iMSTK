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
#include "imstkAPIUtilities.h"

using namespace imstk;

void testViewer()
{
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto sceneTest = sdk->createNewScene("ViewerTest");

    // Plane
    auto planeObj = apiutils::createVisualAnalyticalSceneObject(Geometry::Type::Plane, sceneTest, "VisualPlane", 10);

    // Cube
    auto cubeObj = apiutils::createVisualAnalyticalSceneObject(
        Geometry::Type::Cube, sceneTest, "VisualCube", 0.5, Vec3d(1.0, -1.0, 0.5));
    auto cubeGeom = cubeObj->getVisualGeometry();
    // rotates could be replaced by cubeGeom->setOrientationAxis(1,1,1) (normalized inside)
    cubeGeom->rotate(UP_VECTOR, PI_4, Geometry::TransformType::ApplyToData);
    cubeGeom->rotate(RIGHT_VECTOR, PI_4, Geometry::TransformType::ApplyToData);

    // Sphere
    auto sphereObj = apiutils::createVisualAnalyticalSceneObject(
        Geometry::Type::Sphere, sceneTest, "VisualSphere", 0.3, Vec3d(0, 2., 0));

    // Light (white)
    auto whiteLight = std::make_shared<PointLight>("whiteLight");
    whiteLight->setPosition(Vec3d(5, 8, 5));
    whiteLight->setIntensity(100);
    sceneTest->addLight(whiteLight);

    // Light (red)
    auto colorLight = std::make_shared<SpotLight>("colorLight");
    colorLight->setPosition(Vec3d(4, -3, 1));
    colorLight->setFocalPoint(Vec3d(0, 0, 0));
    colorLight->setColor(Color::Red);
    colorLight->setIntensity(100);
    colorLight->setSpotAngle(1);
    sceneTest->addLight(colorLight);

    // Update Camera
    auto cam1 = sceneTest->getCamera();
    cam1->setPosition(Vec3d(-5.5, 2.5, 32));
    cam1->setFocalPoint(Vec3d(1, 1, 0));

    // Run
    sdk->setActiveScene(sceneTest);
    sdk->startSimulation(SimulationStatus::PAUSED);
}

int main()
{
    testViewer();
    return 0;
}
