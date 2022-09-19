/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
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
#include "imstkTetrahedralMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

const size_t nx = 80 / 2, ny = 40 / 2, nz = 40 / 2;

int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Create a sample scene
    imstkNew<Scene> scene("renderMesh");
    {
        scene->getActiveCamera()->setPosition(0.0, 2.0, 15.0);

        // add scene object for surface object
        imstkNew<SceneObject> surfObj("SurfaceObject");

        auto surfMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/asianDragon/asianDragon.obj");

        // configure and add the render model to the scene object
        imstkNew<RenderMaterial> material;
        material->setDisplayMode(RenderMaterial::DisplayMode::Surface);
        material->setColor(Color::LightGray);
        imstkNew<VisualModel> visualModel;
        visualModel->setGeometry(surfMesh);
        visualModel->setRenderMaterial(material);
        surfObj->addVisualModel(visualModel);

        // add the scene object to the scene
        scene->addSceneObject(surfObj);

        std::shared_ptr<TetrahedralMesh> tetMesh =
            GeometryUtils::createTetrahedralMeshCover(surfMesh, nx, ny, nz);

        // add scene object for surface object
        imstkNew<SceneObject>    volObject("VolObj");
        imstkNew<RenderMaterial> tetMaterial;
        tetMaterial->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
        tetMaterial->setColor(Color::Teal);
        tetMaterial->setPointSize(7.0);
        tetMaterial->setLineWidth(3.0);
        imstkNew<VisualModel> tetVisualModel;
        tetVisualModel->setGeometry(tetMesh);
        tetVisualModel->setRenderMaterial(tetMaterial);
        volObject->addVisualModel(tetVisualModel);

        scene->addSceneObject(volObject);

        // Light
        imstkNew<DirectionalLight> light;
        light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
        light->setIntensity(1);
        scene->addLight("light0", light);
    }

    // Run the simulation
    {
        // Setup a viewer to render in its own thread
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);
        viewer->setBackgroundColors(Color(0.3285, 0.3285, 0.6525), Color(0.13836, 0.13836, 0.2748), true);

        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
        scene->addSceneObject(mouseAndKeyControls);

        // Start viewer running, scene as paused
        driver->start();
    }
}
