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
#include "imstkSceneObject.h"
#include "imstkPlane.h"
#include "imstkCylinder.h"
#include "imstkCube.h"
#include "imstkAPIUtilities.h"
#include "imstkGeometryUtilities.h"

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

    auto coarseTetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg"));
    auto coarseSurfMesh = std::make_shared<SurfaceMesh>();
    coarseTetMesh->extractSurfaceMesh(coarseSurfMesh, true);

    std::shared_ptr<SurfaceMesh> fineSurfaceMesh(std::move(GeometryUtils::loopSubdivideSurfaceMesh(*coarseSurfMesh.get())));

    fineSurfaceMesh->translate(Vec3d(0., -5., 0.), Geometry::TransformType::ConcatenateToTransform);
    coarseSurfMesh->translate(Vec3d(0., 5., 0.), Geometry::TransformType::ConcatenateToTransform);

    auto material0 = std::make_shared<RenderMaterial>();
    material0->setDisplayMode(RenderMaterial::DisplayMode::WIREFRAME_SURFACE);
    auto surfMeshModel0 = std::make_shared<VisualModel>(coarseSurfMesh);
    surfMeshModel0->setRenderMaterial(material0);

    auto sceneObj0 = std::make_shared<VisualObject>("coarse Mesh");
    sceneObj0->addVisualModel(surfMeshModel0);

    scene->addSceneObject(sceneObj0);

    auto material = std::make_shared<RenderMaterial>();
    material->setDebugColor(imstk::Color::Red);
    material->setDisplayMode(RenderMaterial::DisplayMode::WIREFRAME_SURFACE);
    auto surfMeshModel = std::make_shared<VisualModel>(fineSurfaceMesh);
    surfMeshModel->setRenderMaterial(material);

    auto sceneObj = std::make_shared<VisualObject>("fine Mesh");
    sceneObj->addVisualModel(surfMeshModel);

    scene->addSceneObject(sceneObj);

    // Set Camera configuration
    auto cam = scene->getCamera();
    cam->setPosition(Vec3d(0, 12, 12));
    cam->setFocalPoint(Vec3d(0, 0, 0));

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    // Run
    simManager->setActiveScene(scene);
    simManager->start(SimulationStatus::running);

    return 0;
}
