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
#include "imstkLight.h"
#include "imstkAPIUtilities.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkMeshIO.h"
#include "imstkVTKMeshIO.h"
#include "imstkGeometryUtilities.h"

using namespace imstk;

const size_t nx = 80 / 2, ny = 40 / 2, nz = 40 / 2;

int
main()
{
    // Create simulation manager
    auto simManager = std::make_shared<SimulationManager>();

    // Create a sample scene
    auto scene = simManager->createNewScene("renderMesh");
    scene->getCamera()->setPosition(0, 2.0, 15.0);

    // add scene object for surface object
    auto surfaceObject = std::make_shared<VisualObject>("SurfaceObj");

    auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.obj"));

    // configure and add the render model to the scene object
    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::WIREFRAME_SURFACE);
    material->setColor(Color::Red);
    auto surfMeshModel = std::make_shared<VisualModel>(surfMesh);
    surfMeshModel->setRenderMaterial(material);
    surfaceObject->addVisualModel(surfMeshModel);

    // add the scene object to the scene
    scene->addSceneObject(surfaceObject);

    auto tetMesh = GeometryUtils::createTetrahedralMeshCover(*surfMesh, nx, ny, nz);

    // add scene object for surface object
    auto volObject = std::make_shared<VisualObject>("VolObj");
    volObject->setVisualGeometry(tetMesh);

    // add the scene object to the scene
    scene->addSceneObject(volObject);

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    simManager->setActiveScene(scene);
    simManager->getViewer()->setBackgroundColors(Vec3d(0.3285, 0.3285, 0.6525), Vec3d(0.13836, 0.13836, 0.2748), true);
    simManager->start();
}
