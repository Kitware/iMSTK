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
#include "imstkDecalPool.h"
#include "imstkAPIUtilities.h"

using namespace imstk;

///
/// \brief This example demonstrates decal rendering feature.
/// NOTE: Requires enabling Vulkan rendering backend
///
int main()
{
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("Decals");

    // Position camera
    auto cam = scene->getCamera();
    cam->setPosition(0, 3, 6);
    cam->setFocalPoint(0, 0, 0);

    // Decals
    auto decalMaterial = std::make_shared<RenderMaterial>();
    auto decalTexture = std::make_shared<Texture>(iMSTK_DATA_ROOT "/decals/blood_decal.png", Texture::Type::DIFFUSE);
    decalMaterial->addTexture(decalTexture);

    auto decalPool = std::make_shared<DecalPool>();
    auto decalObject = std::make_shared<VisualObject>("Decals");
    auto decalModel = std::make_shared<VisualModel>(decalPool);
    decalModel->setRenderMaterial(decalMaterial);
    decalObject->addVisualModel(decalModel);

    for (int i = -1; i < 2; i++)
    {
        auto decal = decalPool->addDecal();
        decal->setPosition(i, 0, 0.25);
        decal->setRotation(RIGHT_VECTOR, PI_4);
        decal->setScaling(0.5);
    }

    scene->addSceneObject(decalObject);

    // Sphere
    auto sphere = apiutils::createVisualAnalyticalSceneObject(Geometry::Type::Sphere, scene, "sphere", 0.25);
    sphere->getVisualGeometry()->translate(1, 0, 0);

    // Cube
    auto cube = apiutils::createVisualAnalyticalSceneObject(Geometry::Type::Cube, scene, "cube", 0.25);
    cube->getVisualGeometry()->translate(0, 0, 0.1);
    cube->getVisualGeometry()->rotate(UP_VECTOR, PI_4);

    // Plane
    auto plane = apiutils::createVisualAnalyticalSceneObject(Geometry::Type::Plane, scene, "plane", 10);

    // Light
    auto light = std::make_shared<DirectionalLight>("Light");
    light->setIntensity(7);
    light->setColor(Color(1.0, 0.95, 0.8));
    light->setFocalPoint(Vec3d(-1, -1, 0));
    scene->addLight(light);

    // Run
    sdk->setActiveScene(scene);
    sdk->startSimulation(SimulationStatus::PAUSED);

    return 0;
}
