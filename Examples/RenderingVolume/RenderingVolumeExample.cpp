/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkImageData.h"
#include "imstkImageReslice.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneObject.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkVisualModel.h"
#include "imstkVolumeRenderMaterial.h"
#include "imstkVolumeRenderMaterialPresets.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief This example demonstrates the volume renderer
///
int
main()
{
    Logger::startLogger();

    // SDK and Scene
    imstkNew<Scene> scene("VolumeRendering");

    // Read an image
    auto imageData = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "skullVolume.nrrd");

    // Rotate that image
    imstkNew<ImageReslice> reslice;
    reslice->setInputImage(imageData);
    // Rotate 1radian around y
    reslice->setTransform(mat4dRotation(Rotd(1.0, Vec3d(0.0, 1.0, 0.0))));
    reslice->update();

    // Create a visual object in the scene for the volume
    imstkNew<SceneObject> volumeObj("VisualVolume");
    volumeObj->setVisualGeometry(reslice->getOutputImage());
    scene->addSceneObject(volumeObj);

    // Update Camera to position volume close to viewer
    auto cam = scene->getActiveCamera();
    cam->setPosition(108.991, -310.752, 109.795);
    cam->setFocalPoint(41.0774, 35.6817, -33.9048);
    cam->setViewUp(-0.83121, 0.0660308, 0.552024);

    // Setup a viewer to render in its own thread
    imstkNew<VTKViewer> viewer;
    viewer->setActiveScene(scene);
    viewer->setBackgroundColors(Color(0.3285, 0.3285, 0.6525), Color(0.13836, 0.13836, 0.2748), true);

    auto statusTxt = std::make_shared<TextVisualModel>("StatusText");
    statusTxt->setPosition(TextVisualModel::DisplayPosition::UpperLeft);
    statusTxt->setFontSize(30);

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
                              std::shared_ptr<VolumeRenderMaterial> volumeMaterial = VolumeRenderMaterialPresets::getPreset(static_cast<VolumeRenderMaterialPresets::Presets>(currMatId));
                              // Apply the preset to the visual object
                              volumeObj->getVisualModel(0)->setRenderMaterial(volumeMaterial);

                              std::ostringstream ss;
                              ss << "Volume Material Preset: " << imstk::VolumeRenderMaterialPresets::getPresetName(static_cast<VolumeRenderMaterialPresets::Presets>(currMatId));
                              statusTxt->setText(ss.str());
                          }
                      };

    // Run the simulation
    {
        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);
        connect<Event>(viewer, &SceneManager::postUpdate, updateFunc);

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
        mouseAndKeyControls->addComponent(statusTxt);
        scene->addSceneObject(mouseAndKeyControls);

        driver->start();
    }

    return 0;
}
