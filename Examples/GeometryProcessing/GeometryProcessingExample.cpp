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
#include "imstkLight.h"
#include "imstkMeshIO.h"
#include "imstkNew.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneObject.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkSurfaceMeshSubdivide.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVisualModel.h"

using namespace imstk;
using namespace imstk::expiremental;

///
/// \brief This example demonstrates the geometry transforms in imstk
///
int
main()
{
    // simManager and Scene
    imstkNew<SimulationManager> simManager;
    imstkSmartPtr<Scene>        scene = simManager->createNewScene("GeometryTransforms");

    std::shared_ptr<TetrahedralMesh> coarseTetMesh = MeshIO::read<TetrahedralMesh>(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");
    imstkNew<SurfaceMesh>            coarseSurfMesh;
    coarseTetMesh->extractSurfaceMesh(coarseSurfMesh, true);

    imstkNew<SurfaceMeshSubdivide> subdivide;
    subdivide->setInputMesh(coarseSurfMesh);
    subdivide->setSubdivisionType(SurfaceMeshSubdivide::Type::LOOP);
    subdivide->setNumberOfSubdivisions(1);
    subdivide->update();

    std::shared_ptr<SurfaceMesh> fineSurfaceMesh = std::dynamic_pointer_cast<SurfaceMesh>(subdivide->getOutput());

    fineSurfaceMesh->translate(Vec3d(0.0, -5.0, 0.0), Geometry::TransformType::ConcatenateToTransform);
    coarseSurfMesh->translate(Vec3d(0.0, 5.0, 0.0), Geometry::TransformType::ConcatenateToTransform);

    imstkNew<RenderMaterial> material0;
    material0->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material0->setPointSize(10.);
    material0->setLineWidth(4.);
    material0->setEdgeColor(Color::Color::Orange);
    imstkNew<VisualModel> surfMeshModel0(coarseSurfMesh.get());
    surfMeshModel0->setRenderMaterial(material0);

    imstkNew<VisualObject> sceneObj0("coarse Mesh");
    sceneObj0->addVisualModel(surfMeshModel0);

    scene->addSceneObject(sceneObj0);

    imstkNew<RenderMaterial> material;
    material->setColor(imstk::Color::Red);
    material->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
    material->setPointSize(6.);
    material->setLineWidth(1.);
    imstkNew<VisualModel> surfMeshModel(fineSurfaceMesh);
    surfMeshModel->setRenderMaterial(material);

    imstkNew<VisualObject> sceneObj("fine Mesh");
    sceneObj->addVisualModel(surfMeshModel);

    scene->addSceneObject(sceneObj);

    // Set Camera configuration
    auto cam = scene->getCamera();
    cam->setPosition(Vec3d(0, 12, 12));
    cam->setFocalPoint(Vec3d(0, 0, 0));

    // Light
    imstkNew<DirectionalLight> light("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    // Run
    simManager->setActiveScene(scene);
    simManager->start(SimulationStatus::Running);

    return 0;
}
