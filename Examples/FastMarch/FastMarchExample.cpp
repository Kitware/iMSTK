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
#include "imstkDataArray.h"
#include "imstkFastMarch.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLogger.h"
#include "imstkMeshIO.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneObject.h"
#include "imstkSimulationManager.h"
#include "imstkVisualModel.h"
#include "imstkVolumeRenderMaterial.h"
#include "imstkVTKViewer.h"

#include <vtkVolumeProperty.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>

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

    // Create some blank image
    imstkNew<ImageData> image;
    image->allocate(IMSTK_DOUBLE, 1, Vec3i(50, 50, 50));
    std::shared_ptr<DataArray<double>> scalars = std::dynamic_pointer_cast<DataArray<double>>(image->getScalars());
    scalars->fill(0.0);

    // Seed voxels
    imstkNew<FastMarch> fastMarch;
    fastMarch->setDistThreshold(5.0);
    fastMarch->setImage(image);
    std::vector<Vec3i> seeds = { Vec3i(25, 25, 25) };
    fastMarch->setSeeds(seeds);
    fastMarch->solve();

    // Create a visual object in the scene for the volume
    imstkNew<SceneObject> volumeObj("VisualVolume");
    volumeObj->setVisualGeometry(image);
    scene->addSceneObject(volumeObj);

    imstkNew<VolumeRenderMaterial>   volumeMaterial;
    vtkNew<vtkColorTransferFunction> colorFunc;
    colorFunc->AddRGBPoint(0.0, 1.0, 0.0, 0.0);
    colorFunc->AddRGBPoint(8.0, 0.0, 0.0, 1.0);
    volumeMaterial->getVolumeProperty()->SetColor(colorFunc);
    vtkNew<vtkPiecewiseFunction> opacityFunc;
    opacityFunc->AddPoint(0.0, 0.0);
    opacityFunc->AddPoint(1.0, 1.0);
    volumeMaterial->getVolumeProperty()->SetScalarOpacity(opacityFunc);
    // Apply the preset to the visual object
    volumeObj->getVisualModel(0)->setRenderMaterial(volumeMaterial);

    scene->getActiveCamera()->setPosition(Vec3d(0.0, -100.0, -100.0));
    scene->getActiveCamera()->setFocalPoint(Vec3d(25.0, 25.0, 25.0));
    scene->getActiveCamera()->setViewUp(Vec3d(0.0, 1.0, 0.0));

    // Run the simulation
    {
        imstkNew<VTKViewer> viewer("Viewer");
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager("Scene Manager");
        sceneManager->setActiveScene(scene);
        sceneManager->setExecutionType(Module::ExecutionType::ADAPTIVE);

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

    MeshIO::write(image, "results.nii");

    return 0;
}
