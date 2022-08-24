/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkDirectionalLight.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLogger.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneObject.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkSurfaceMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief Demonstrates PBR rendering with SSAO
///
int
main()
{
    // Write log to stdout and file
    Logger::startLogger();

    imstkNew<Scene> scene("RenderingHead");
    {
        // Head mesh
        auto surfaceMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/head/head_revised.obj");

        imstkNew<RenderMaterial> material;
        material->setDisplayMode(RenderMaterial::DisplayMode::Surface);
        material->setShadingModel(RenderMaterial::ShadingModel::PBR);
        material->addTexture(std::make_shared<Texture>(iMSTK_DATA_ROOT "head/HeadTexture_BaseColor.png", Texture::Type::Diffuse));
        material->addTexture(std::make_shared<Texture>(iMSTK_DATA_ROOT "head/HeadTexture_Normal.png", Texture::Type::Normal));
        material->addTexture(std::make_shared<Texture>(iMSTK_DATA_ROOT "head/HeadTexture_AO.png", Texture::Type::AmbientOcclusion));
        material->setRecomputeVertexNormals(false);

        imstkNew<VisualModel> surfMeshModel;
        surfMeshModel->setGeometry(surfaceMesh);
        surfMeshModel->setRenderMaterial(material);

        imstkNew<SceneObject> headObject("head");
        headObject->addVisualModel(surfMeshModel);

        scene->addSceneObject(headObject);

        // Position camera
        scene->getActiveCamera()->setPosition(0.0, 0.25, 0.6);
        scene->getActiveCamera()->setFocalPoint(0.0, 0.25, 0.0);

        // Lights
        imstkNew<DirectionalLight> dirLight;
        dirLight->setIntensity(10.0);
        dirLight->setColor(Color(1.0, 0.95, 0.8));
        scene->addLight("directionalLight", dirLight);
    }

    // Run the simulation
    {
        // Setup a viewer to render in its own thread
        imstkNew<VTKViewer> viewer;
        viewer->setVtkLoggerMode(AbstractVTKViewer::VTKLoggerMode::MUTE);
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControlEntity(driver);
        scene->addSceneObject(mouseAndKeyControls);

        // Enable SSAO
        Vec3d l, u;
        scene->computeBoundingBox(l, u);
        const double sceneSize = (u - l).norm();

        auto rendConfig = std::make_shared<RendererConfig>();
        rendConfig->m_ssaoConfig.m_enableSSAO = true;
        rendConfig->m_ssaoConfig.m_SSAOBlur   = true;
        rendConfig->m_ssaoConfig.m_SSAORadius = 10.0 * sceneSize;
        rendConfig->m_ssaoConfig.m_SSAOBias   = 0.01 * sceneSize;
        rendConfig->m_ssaoConfig.m_KernelSize = 128;

        viewer->getActiveRenderer()->setConfig(rendConfig);

        driver->start();
    }

    return 0;
}
