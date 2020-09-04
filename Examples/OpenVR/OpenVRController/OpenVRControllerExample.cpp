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
#include "imstkLight.h"
#include "imstkLogger.h"
#include "imstkMeshIO.h"
#include "imstkNew.h"
#include "imstkOpenVRDeviceClient.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneObject.h"
#include "imstkSceneObjectController.h"
#include "imstkSurfaceMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKOpenVRViewer.h"

using namespace imstk;

///
/// \brief This example demonstrates rendering and controlling a SceneObject with OpenVR
///
int
main()
{
    // Write log to stdout and file
    Logger::startLogger();

    // Setup the scene
    imstkNew<Scene>       scene("Rendering");
    imstkNew<SceneObject> sceneObject("Scalpel");
    {
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
        material->setMetalness(0.9f);
        material->setRoughness(0.2f);
        material->addTexture(toolDiffuseTexture);
        surfMeshModel->setRenderMaterial(material);

        sceneObject->addVisualModel(surfMeshModel);

        scene->addSceneObject(sceneObject);

        // Lights
        imstkNew<DirectionalLight> dirLight("DirLight");
        dirLight->setIntensity(4);
        dirLight->setColor(Color(1.0, 0.95, 0.8));
        dirLight->setCastsShadow(true);
        dirLight->setShadowRange(1.5);
        scene->addLight(dirLight);
    }

    {
        // Add a module to run the viewer
        imstkNew<VTKOpenVRViewer> viewer("VRViewer");
        viewer->setActiveScene(scene);

        // Add a module to run the scene
        imstkNew<SceneManager> sceneManager("Scene Manager");
        sceneManager->setActiveScene(scene);
        viewer->addChildThread(sceneManager); // Start/stop scene with the view

        // Add a VR controller for the sceneobject
        imstkNew<SceneObjectController> controller(sceneObject, viewer->getVRDeviceClient(OPENVR_RIGHT_CONTROLLER));
        scene->addController(controller);

        // Start running
        viewer->requestStatus(ThreadStatus::Running);
        viewer->start();
    }

    return 0;
}