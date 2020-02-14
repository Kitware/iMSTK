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

#include "imstkAPIUtilities.h"
#include "imstkMeshIO.h"
#include "imstkSimulationManager.h"
#include "imstkVolumeRenderMaterial.h"
#include "imstkVolumeRenderMaterialPresets.h"

using namespace imstk;

///
/// \brief This example demonstrates configuring the volume renderer.
///
int
main()
{
    // SDK and Scene
    auto sdk       = std::make_shared<SimulationManager>();
    auto sceneTest = sdk->createNewScene("VolumeRendering");
    sdk->setActiveScene(sceneTest);

    // Use MeshIO to read the image dataset
    auto imageData = imstk::MeshIO::read(iMSTK_DATA_ROOT "DB_CBCT_transform_ASCII.nrrd");
    // Create a visual object in the scene for the volume
    std::shared_ptr<imstk::VisualObject> volumeObj =
        std::make_shared<imstk::VisualObject>("VisualVolume");
    volumeObj->setVisualGeometry(imageData);
    sceneTest->addSceneObject(volumeObj);

    // Update Camera to position volume close to viewer
    auto cam1 = sceneTest->getCamera();
    cam1->setPosition(Vec3d(0, -200, -50));
    cam1->setFocalPoint(Vec3d(0, 0, -50));
    cam1->setViewUp(Vec3d(0.02, 0.4, 0.9));

    int count = 0;
    // Get VTK Renderer
    auto viewer     = std::dynamic_pointer_cast<VTKViewer>(sdk->getViewer());
    auto renderer   = std::dynamic_pointer_cast<VTKRenderer>(viewer->getActiveRenderer());
    auto updateFunc =
        [&](Module*) {
            if (count % 2)
            {
                // Change the render material every other frame
                ++count;
                return;
            }
            // Setting the renderer mode removes vtk actors
            // renderer->setMode(VTKRenderer::SIMULATION, false);
            if (count > 50)
            {
                // There are a total of 25 presets thus far.
                count = 0;
            }
            // Change view background to black every other frame
            renderer->updateBackground(Vec3d(0, 0, 0));
            std::cout << "Displaying with volume material preset: " << count / 2 << std::endl;
            // Query for a volume material preset
            auto mat = imstk::VolumeRenderMaterialPresets::getPreset(count / 2);
            // Apply the preset to the visual object
            volumeObj->getVisualModel(0)->setRenderMaterial(mat);
            ++count;
        };
    auto postUpdateFunc =
        [&](Module*) {
            // Delay to show the past render
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            // Change view background to gray every other frame
            renderer->updateBackground(Vec3d(0.7, 0.7, 0.7));
        };
    sdk->getSceneManager(sceneTest)->setPreUpdateCallback(updateFunc);
    sdk->getSceneManager(sceneTest)->setPostUpdateCallback(postUpdateFunc);
    // Run
    sdk->startSimulation(SimulationStatus::RUNNING);

    return 0;
}
