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

#include "imstkCylinder.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkMeshIO.h"
//#include "imstkVTKMeshIO.h"
#include "imstkGeometryUtilities.h"

#include "imstkSimulationManager.h"
#include "imstkPlane.h"
#include "imstkSphere.h"

using namespace imstk;

///
/// \brief Generate cylinder
///
std::vector<std::shared_ptr<CollidingObject>> generateSolidsScene1(const std::shared_ptr<Scene>& scene)
{
  std::vector<std::shared_ptr<CollidingObject>> solids;
    auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(MeshIO::read(iMSTK_DATA_ROOT "/cylinder/cylinder.stl"));

    // visualize
    auto surfaceObject = std::make_shared<VisualObject>("SurfaceObj");
    auto visualModel = std::make_shared<VisualModel>(surfMesh);
    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
    material->setColor(Color(Color::Red, 0.1));
    //auto color = material->getColor();
    auto surfMeshModel = std::make_shared<VisualModel>(surfMesh);
    surfMeshModel->setRenderMaterial(material);
    surfaceObject->addVisualModel(surfMeshModel);
    // add the scene object to the scene
    scene->addSceneObject(surfaceObject);
  return solids;
}


std::vector<std::shared_ptr<CollidingObject>> generateSolidsScene2(const std::shared_ptr<Scene>& scene)
{
  std::vector<std::shared_ptr<CollidingObject>> solids;
  return solids;
}

std::vector<std::shared_ptr<CollidingObject>> generateSolids(const std::shared_ptr<Scene>& scene)
{
  switch (SCENE_ID)
  {
  case 1:
    return generateSolidsScene1(scene);
  case 2 :
    return generateSolidsScene2(scene);
  default:
    LOG(FATAL) << "Invalid scene index";
    return {}; // To avoid warning
  }
}