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
#include "imstkCamera.h"
#include "imstkIBLProbe.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLight.h"
#include "imstkMeshIO.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneObject.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"
#include "imstkLogger.h"

using namespace imstk;

///
/// \brief This example demonstrates configuring the renderer
/// objects, lights etc.
///
int
main()
{
    // Write log to stdout and file
    Logger::startLogger();

    imstkNew<Scene> scene("Rendering");
    {
        // Add IBL Probe
        imstkNew<IBLProbe> globalIBLProbe(
            iMSTK_DATA_ROOT "/IBL/roomIrradiance.dds",
            iMSTK_DATA_ROOT "/IBL/roomRadiance.dds",
            iMSTK_DATA_ROOT "/IBL/roomBRDF.png");
        scene->setGlobalIBLProbe(globalIBLProbe);

        // Head mesh
        auto surfaceMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/head/head_revised.obj");

        imstkNew<RenderMaterial> material;
        material->setDisplayMode(RenderMaterial::DisplayMode::Surface);
        material->setShadingModel(RenderMaterial::ShadingModel::PBR);
        material->addTexture(std::make_shared<Texture>(iMSTK_DATA_ROOT "head/HeadTexture_BaseColor.png", Texture::Type::Diffuse));
        material->addTexture(std::make_shared<Texture>(iMSTK_DATA_ROOT "head/HeadTexture_Normal.png", Texture::Type::Normal));
        material->addTexture(std::make_shared<Texture>(iMSTK_DATA_ROOT "head/HeadTexture_AO.png", Texture::Type::AmbientOcclusion));
        material->setRecomputeVertexNormals(false);
        material->setReceivesShadows(true);
        material->setCastsShadows(true);

        imstkNew<VisualModel> surfMeshModel(surfaceMesh);
        surfMeshModel->setRenderMaterial(material);

        imstkNew<SceneObject> headObject("head");
        headObject->addVisualModel(surfMeshModel);

        scene->addSceneObject(headObject);

        // Position camera
        scene->getActiveCamera()->setPosition(0.0, 0.25, 0.6);
        scene->getActiveCamera()->setFocalPoint(0.0, 0.25, 0.0);

        // Lights
        imstkNew<DirectionalLight> dirLight("DirectionalLight");
        dirLight->setIntensity(4.0);
        dirLight->setColor(Color(1.0, 0.95, 0.8));
        dirLight->setCastsShadow(true);
        dirLight->setShadowRange(1.5);
        scene->addLight(dirLight);

        imstkNew<PointLight> pointLight("PointLight");
        pointLight->setIntensity(0.1);
        pointLight->setPosition(0.1, 0.2, 0.5);
        //scene->addLight(pointLight);

        // Plane
        auto                     planeObj = apiutils::createVisualAnalyticalSceneObject("Plane", scene, "VisualPlane", Vec3d(10.0, 10.0, 10.0));
        imstkNew<RenderMaterial> planeMaterial;
        planeMaterial->setColor(Color::LightGray);
        planeMaterial->setReceivesShadows(true);
        planeMaterial->setCastsShadows(true);
        planeObj->getVisualModel(0)->setRenderMaterial(planeMaterial);
    }

    // Run the simulation
    {
        // Setup a viewer to render in its own thread
        imstkNew<VTKViewer> viewer("Viewer");
        viewer->setVtkLoggerMode(AbstractVTKViewer::VTKLoggerMode::MUTE);
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager("Scene Manager");
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

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
