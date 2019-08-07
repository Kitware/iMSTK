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
#include "imstkMeshIO.h"
#include "imstkVolumetricMesh.h"

using namespace imstk;

///
/// \brief This example shows how to read .msh and .veg files
///
int
main()
{
    // SDK and Scene
    auto sdk   = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("MshAndVegaIO");

    // Load a volumetric mesh (from .msh file)
    std::string ifile    = iMSTK_DATA_ROOT "/liver/liver.msh";
    auto        volMeshA = MeshIO::read(ifile);

    // Extract surface mesh
    auto volumeMeshA = std::dynamic_pointer_cast<VolumetricMesh>(volMeshA); // change to any volumetric mesh above
    volumeMeshA->computeAttachedSurfaceMesh();
    auto surfaceMeshA = volumeMeshA->getAttachedSurfaceMesh();
    surfaceMeshA->correctWindingOrder();
    surfaceMeshA->flipNormals();

    // Create object A
    auto objectA = std::make_shared<VisualObject>("meshObjectMSH");
    objectA->setVisualGeometry(surfaceMeshA);

    // Write a .veg file
    std::string ofile       = iMSTK_DATA_ROOT "/liver/liver.veg";
    auto        writeStatus = MeshIO::write(volMeshA, ofile);
    std::cout << "------------------------------Summary----------------------------------------------------\n";
    std::cout << "Following file conversion: " << ((writeStatus) ? "Success \n" : "Failure \n");
    std::cout << "\n Input mesh file : \n" << ifile << std::endl;
    std::cout << "\n Output mesh file: \n" << ofile << std::endl;

    // Read the above written veg file
    auto volMeshB = MeshIO::read(ofile);

    // Extract surface mesh
    auto volumeMeshB = std::dynamic_pointer_cast<VolumetricMesh>(volMeshB); // change to any volumetric mesh above
    volumeMeshB->computeAttachedSurfaceMesh();
    auto surfaceMeshB = volumeMeshB->getAttachedSurfaceMesh();
    surfaceMeshB->correctWindingOrder();
    surfaceMeshB->flipNormals();

    // Create object B
    auto objectB = std::make_shared<VisualObject>("meshObjectVEGA");
    surfaceMeshB->translate(Vec3d(10, 0, 0), Geometry::TransformType::ApplyToData);
    objectB->setVisualGeometry(surfaceMeshB);

    // Add objects to the scene
    scene->addSceneObject(objectA);
    scene->addSceneObject(objectB);

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
