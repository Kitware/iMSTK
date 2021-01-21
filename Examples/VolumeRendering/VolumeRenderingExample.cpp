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
#include "imstkImageData.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMeshIO.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneObject.h"
#include "imstkSimulationManager.h"
#include "imstkVisualModel.h"
#include "imstkVolumeRenderMaterial.h"
#include "imstkVolumeRenderMaterialPresets.h"
#include "imstkVTKRenderer.h"
#include "imstkVTKTextStatusManager.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief This example demonstrates the volume renderer
///
int
main()
{
    // SDK and Scene
    imstkNew<Scene> scene("VolumeRendering");

    // Create a visual object in the scene for the volume
    imstkNew<VisualObject> volumeObj("VisualVolume");
    auto                   imageData = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "skullVolume.nrrd");
    volumeObj->setVisualGeometry(imageData);
    scene->addSceneObject(volumeObj);

    // Update Camera to position volume close to viewer
    auto cam = scene->getActiveCamera();
    cam->setPosition(Vec3d(0.0, -200.0, -100.0) * 2.0);
    cam->setFocalPoint(Vec3d(0.0, 0.0, -50.0));
    cam->setViewUp(Vec3d(0.02, 0.4, 0.9));

    // Setup a viewer to render in its own thread
    imstkNew<VTKViewer> viewer("Viewer");
    viewer->setActiveScene(scene);

    auto renderer = std::dynamic_pointer_cast<VTKRenderer>(viewer->getActiveRenderer());
    renderer->updateBackground(Vec3d(0.3285, 0.3285, 0.6525), Vec3d(0.13836, 0.13836, 0.2748), true);

    auto statusManager = viewer->getTextStatusManager();
    statusManager->setStatusFontSize(VTKTextStatusManager::StatusType::Custom, 30);
    statusManager->setStatusDisplayCorner(VTKTextStatusManager::StatusType::Custom, VTKTextStatusManager::DisplayCorner::UpperLeft);

    StopWatch timer;
    timer.start();

    int  currMatId  = 0;
    int  prevMatId  = -1;
    auto updateFunc = [&](Event*)
                      {
                          prevMatId = currMatId;
                          currMatId = static_cast<int>(timer.getTimeElapsed() / 1000.0 / 2.0) % 25;

                          if (currMatId != prevMatId)
                          {
                              // Change view background to black every other frame
                              std::cout << "Displaying with volume material preset: " << currMatId << std::endl;
                              // Query for a volume material preset
                              std::shared_ptr<VolumeRenderMaterial> volumeMaterial = VolumeRenderMaterialPresets::getPreset(currMatId);
                              // Apply the preset to the visual object
                              volumeObj->getVisualModel(0)->setRenderMaterial(volumeMaterial);

                              std::ostringstream ss;
                              ss << "Volume Material Preset: " << imstk::VolumeRenderMaterialPresets::getPresetName(currMatId);
                              statusManager->setCustomStatus(ss.str());
                          }
                      };

    // Run the simulation
    {
        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager("Scene Manager");
        sceneManager->setActiveScene(scene);
        connect<Event>(viewer, EventType::PostUpdate, updateFunc);

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);

        // Add mouse and keyboard controls to the viewer
        {
            imstkNew<MouseSceneControl> mouseControl(viewer->getMouseDevice());
            mouseControl->setSceneManager(sceneManager);
            viewer->addControl(mouseControl);

            imstkNew<KeyboardSceneControl> keyControl(viewer->getKeyboardDevice());
            keyControl->setSceneManager(sceneManager);
            keyControl->setModuleDriver(driver);
            viewer->addControl(keyControl);
        }

        driver->start();
    }

    return 0;
}
