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
#include "imstkSceneManager.h"
#include "imstkSceneObject.h"
#include "imstkCamera.h"
#include "imstkLight.h"
#include "imstkPlane.h"
#include "imstkCylinder.h"
#include "imstkCube.h"
#include "imstkAPIUtilities.h"
#include "imstkScene.h"

using namespace imstk;

///
/// \brief This example demonstrates the geometry transforms in imstk
///
int
main()
{
    // simManager and Scene
    auto simManager = std::make_shared<SimulationManager>();
    auto scene      = simManager->createNewScene("GeometryTransforms");

    auto sceneObj = apiutils::createAndAddVisualSceneObject(scene, iMSTK_DATA_ROOT "/asianDragon/asianDragon.obj", "Dragon");

    CHECK(sceneObj != nullptr) << "ERROR: Unable to create scene object";

    auto surfaceMesh = sceneObj->getVisualGeometry();
    surfaceMesh->scale(5., Geometry::TransformType::ConcatenateToTransform);

    //  Plane
    auto planeGeom = std::make_shared<Plane>();
    planeGeom->scale(80, Geometry::TransformType::ConcatenateToTransform);
    planeGeom->translate(0, -20, 0, Geometry::TransformType::ConcatenateToTransform);
    planeGeom->rotate(Vec3d(0, 1., 0), PI / 4, Geometry::TransformType::ConcatenateToTransform);

    auto planeObj = std::make_shared<VisualObject>("Plane");
    planeObj->setVisualGeometry(planeGeom);
    scene->addSceneObject(planeObj);

    //  Cube
    auto cubeGeom = std::make_shared<Cube>();
    cubeGeom->setWidth(20.);
    cubeGeom->scale(0.5, Geometry::TransformType::ConcatenateToTransform);
    cubeGeom->rotate(Vec3d(1., 1., 0), PI / 4, Geometry::TransformType::ApplyToData);

    auto cubeObj = std::make_shared<VisualObject>("Cube");
    cubeObj->setVisualGeometry(cubeGeom);
    scene->addSceneObject(cubeObj);

    //  Cylinder
    auto CylinderGeom = std::make_shared<Cylinder>();
    CylinderGeom->setRadius(4.);
    CylinderGeom->setLength(8.);
    CylinderGeom->scale(0.4, Geometry::TransformType::ConcatenateToTransform);
    CylinderGeom->rotate(Vec3d(1., 1., 0), PI / 2, Geometry::TransformType::ApplyToData);

    auto CylinderObj = std::make_shared<VisualObject>("Cylinder");
    CylinderObj->setVisualGeometry(CylinderGeom);
    scene->addSceneObject(CylinderObj);

    // Rotate the dragon every frame
    auto rotateFunc =
        [&surfaceMesh](Module* module)
        {
            surfaceMesh->rotate(Vec3d(1., 0, 0), PI / 1000, Geometry::TransformType::ApplyToData);
        };
    simManager->getSceneManager(scene)->setPostUpdateCallback(rotateFunc);

    // Set Camera configuration
    auto cam = scene->getCamera();
    cam->setPosition(Vec3d(0, 30, 30));
    cam->setFocalPoint(Vec3d(0, 0, 0));

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    // Run
    simManager->setActiveScene(scene);
    simManager->start(SimulationStatus::Running);

    return 0;
}
