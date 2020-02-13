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

    std::shared_ptr<imstk::VisualObject> volumeObj =
        std::make_shared<imstk::VisualObject>("VisualVolume");
    auto imageData = imstk::MeshIO::read(iMSTK_DATA_ROOT "DB_CBCT_transform_ASCII.nrrd");
    volumeObj->setVisualGeometry(imageData);
    volumeObj->getVisualModel(0)->setRenderMaterial(
        imstk::VolumeRenderMaterialPresets::getPreset(
            imstk::VolumeRenderMaterialPresets::CT_BONE));
    sceneTest->addSceneObject(volumeObj);

    // Update Camera
    auto cam1 = sceneTest->getCamera();
    cam1->setPosition(Vec3d(-5.5, 2.5, 32));
    cam1->setFocalPoint(Vec3d(1, 1, 0));

    // Run
    sdk->setActiveScene(sceneTest);
    sdk->startSimulation(SimulationStatus::PAUSED);

    return 0;
}
