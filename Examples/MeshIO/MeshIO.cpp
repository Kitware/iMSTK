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
#include  "imstkVolumetricMesh.h"
#include "imstkMeshIO.h"

using namespace imstk;

int main()
{
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("ReadMesh");

    // Read surface mesh
    /*auto objMesh = MeshIO::read(iMSTK_DATA_ROOT"/asianDragon/asianDragon.obj");
    auto plyMesh = MeshIO::read(iMSTK_DATA_ROOT"/cube/cube.ply");
    auto stlMesh = MeshIO::read(iMSTK_DATA_ROOT"/cube/cube.stl");
    auto vtkMesh = MeshIO::read(iMSTK_DATA_ROOT"/cube/cube.vtk");
    auto vtpMesh = MeshIO::read(iMSTK_DATA_ROOT"/cube/cube.vtp");*/

    // Read volumetricMesh
    //auto vtkMesh2 = MeshIO::read(iMSTK_DATA_ROOT"/nidus/nidus.vtk");
    auto vegaMesh = MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");

    // Extract surface mesh
    auto volumeMesh = std::dynamic_pointer_cast<VolumetricMesh>(vegaMesh); // change to any volumetric mesh above
    volumeMesh->computeAttachedSurfaceMesh();
    auto surfaceMesh = volumeMesh->getAttachedSurfaceMesh();

    // Create object and add to scene
    auto object = std::make_shared<VisualObject>("meshObject");
    object->setVisualGeometry(surfaceMesh); // change to any mesh created above
    scene->addSceneObject(object);

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    // Run
    sdk->setActiveScene(scene);
    sdk->startSimulation(SimulationStatus::PAUSED);
    return 0;
}
