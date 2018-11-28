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
#include "imstkCollidingObject.h"
#include "imstkMeshIO.h"

using namespace imstk;

///
/// \brief This example demonstrates the continuous collision detection feature
///
int main()
{
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("CCD");

    auto mesh1 = MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.obj");
    auto mesh2 = MeshIO::read(iMSTK_DATA_ROOT "/textured_organs/heart.obj");

    // Obj1
    auto obj1 = std::make_shared<CollidingObject>("obj1");
    auto objModel1 = std::make_shared<VisualModel>(mesh1);
    obj1->addVisualModel(objModel1);
    obj1->setCollidingGeometry(mesh1);
    scene->addSceneObject(obj1);

    // set configure Obj1 rendering
    auto matObj1 = std::make_shared<RenderMaterial>();
    matObj1->setBackFaceCulling(false);
    matObj1->setColor(Color::Green);
    matObj1->setDisplayMode(RenderMaterial::DisplayMode::WIREFRAME_SURFACE);
    objModel1->setRenderMaterial(matObj1);

    // Obj2
    auto obj2 = std::make_shared<CollidingObject>("obj2");
    auto objModel2 = std::make_shared<VisualModel>(mesh2);
    obj2->setVisualGeometry(mesh2);
    obj2->setCollidingGeometry(mesh2);
    scene->addSceneObject(obj2);

    // set configure Obj2 rendering
    auto matObj2 = std::make_shared<RenderMaterial>();
    matObj2->setBackFaceCulling(false);
    matObj2->setColor(Color::Green);
    matObj2->setDisplayMode(RenderMaterial::DisplayMode::WIREFRAME_SURFACE);
    objModel2->setRenderMaterial(matObj2);

    // Collisions
    auto colGraph = scene->getCollisionGraph();
    colGraph->addInteractionPair(obj1, obj2,
        CollisionDetection::Type::MeshToMesh,
        CollisionHandling::Type::None,
        CollisionHandling::Type::None);

    // Rotate the obj1 every frame
    auto rotateFunc = [&](Module* module)
                      {
                          mesh1->rotate(Vec3d(1., 0, 0), PI / 1000, Geometry::TransformType::ApplyToData);
                      };
    sdk->getSceneManager(scene)->setPostUpdateCallback(rotateFunc);

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    // set the position of the camera
    scene->getCamera()->setPosition(0., 0., 10);

    // Run
    sdk->setActiveScene(scene);
    sdk->startSimulation(SimulationStatus::PAUSED);

    return 0;
}
