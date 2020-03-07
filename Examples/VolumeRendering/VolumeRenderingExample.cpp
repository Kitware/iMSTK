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

// imstk
#include "imstkAPIUtilities.h"
#include "imstkMeshIO.h"
#include "imstkSimulationManager.h"
#include "imstkVolumeRenderMaterial.h"
#include "imstkVolumeRenderMaterialPresets.h"
#include "imstkVTKTextStatusManager.h"

// STL
#include <sstream>
#include <string>

using namespace imstk;

///
/// \brief This example demonstrates the volume renderer
///
int
main()
{
    // SDK and Scene
    auto simManager = std::make_shared<SimulationManager>();
    auto scene      = simManager->createNewScene("VolumeRendering");
    simManager->setActiveScene(scene);

    // Use MeshIO to read the image dataset
    auto imageData = imstk::MeshIO::read(iMSTK_DATA_ROOT "skullVolume.nrrd");
    // Create a visual object in the scene for the volume
    auto volumeObj = std::make_shared<imstk::VisualObject>("VisualVolume");
    volumeObj->setVisualGeometry(imageData);
    scene->addSceneObject(volumeObj);

    // Update Camera to position volume close to viewer
    auto cam = scene->getCamera();
    cam->setPosition(Vec3d(0, -200, -50));
    cam->setFocalPoint(Vec3d(0, 0, -50));
    cam->setViewUp(Vec3d(0.02, 0.4, 0.9));

    int count = 0;
    // Get VTK Renderer
    auto viewer   = std::dynamic_pointer_cast<VTKViewer>(simManager->getViewer());
    auto renderer = std::dynamic_pointer_cast<VTKRenderer>(viewer->getActiveRenderer());
    renderer->updateBackground(Vec3d(0.3285, 0.3285, 0.6525), Vec3d(0.13836, 0.13836, 0.2748), true);

    auto statusManager = viewer->getTextStatusManager();
    statusManager->setStatusFontSize(VTKTextStatusManager::Custom, 30);
    statusManager->setStatusDisplayCorner(VTKTextStatusManager::Custom, VTKTextStatusManager::DisplayCorner::UpperLeft);

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
            std::cout << "Displaying with volume material preset: " << count / 2 << std::endl;
            // Query for a volume material preset
            auto mat = imstk::VolumeRenderMaterialPresets::getPreset(count / 2);
            // Apply the preset to the visual object
            volumeObj->getVisualModel(0)->setRenderMaterial(mat);

            std::ostringstream ss;
            ss << "Volume Material Preset: " << imstk::VolumeRenderMaterialPresets::getPresetName(count / 2);
            statusManager->setCustomStatus(ss.str());

            ++count;
            // Delay to show the past frame
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        };
    simManager->getSceneManager(scene)->setPreUpdateCallback(updateFunc);
    // Run
    simManager->start();

    return 0;
}
