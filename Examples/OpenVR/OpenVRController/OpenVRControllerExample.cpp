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
#include "imstkDeviceTracker.h"
#include "imstkIBLProbe.h"
#include "imstkLight.h"
#include "imstkMeshIO.h"
#include "imstkNew.h"
#include "imstkOpenVRDeviceClient.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneObject.h"
#include "imstkSceneObjectController.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;
using namespace imstk::expiremental;

///
/// \brief This example demonstrates rendering and controlling a SceneObject with OpenVR
///
int
main()
{
    // simManager and Scene
    imstkNew<SimulationManager> simManager;
    simManager->getConfig()->enableVR = true; // Controllers cannot function without HMD
    std::shared_ptr<Scene> scene      = simManager->createNewScene("Rendering");

    std::shared_ptr<SurfaceMesh> toolMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Scalpel Blade10/Scalpel Blade10.obj");
    auto                         toolDiffuseTexture =
        std::make_shared<Texture>(iMSTK_DATA_ROOT "/Surgical Instruments/Scalpel Blade10/Scalpel Blade10 Albedo.png", Texture::Type::Diffuse);
    toolMesh->translate(0.0, 0.0, 1.0, Geometry::TransformType::ApplyToData);
    toolMesh->rotate(Vec3d(0.0, 1.0, 0.0), 3.14, Geometry::TransformType::ApplyToData);
    toolMesh->rotate(Vec3d(1.0, 0.0, 0.0), -1.57, Geometry::TransformType::ApplyToData);
    toolMesh->scale(0.06, Geometry::TransformType::ApplyToData);

    imstkNew<VisualModel>    surfMeshModel(toolMesh);
    imstkNew<RenderMaterial> material;
    material->setDisplayMode(RenderMaterial::DisplayMode::Surface);
    material->setShadingModel(RenderMaterial::ShadingModel::PBR);
    material->addTexture(toolDiffuseTexture);
    surfMeshModel->setRenderMaterial(material);

    imstkNew<SceneObject> sceneObject("scalpel");
    sceneObject->addVisualModel(surfMeshModel);

    scene->addSceneObject(sceneObject);

    // Position camera
    std::shared_ptr<Camera> cam = scene->getCamera();
    cam->setPosition(0, 0.25, 0.6);
    cam->setFocalPoint(0, 0.25, 0);

    // Lights
    imstkNew<DirectionalLight> dirLight("DirLight");
    dirLight->setIntensity(4);
    dirLight->setColor(Color(1.0, 0.95, 0.8));
    dirLight->setCastsShadow(true);
    dirLight->setShadowRange(1.5);
    scene->addLight(dirLight);

    // The viewer must be setup before acquiring the VR device
    simManager->setActiveScene(scene);

    // The controller actually applies the transforms
    // \todo what the api should look like
    /*imstkNew<SceneObjectController> controller(headObject, rightControllerClient);
    scene->addObjectController(controller);*/
    imstkNew<SceneObjectController> controller(sceneObject, std::make_shared<DeviceTracker>(simManager->getViewer()->getVRDeviceClient(OPENVR_RIGHT_CONTROLLER)));
    scene->addObjectController(controller);

    simManager->start(SimulationStatus::Running);

    return 0;
}
