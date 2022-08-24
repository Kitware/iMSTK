/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCylinder.h"
#include "imstkDirectionalLight.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLogger.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkOrientedBox.h"
#include "imstkPlane.h"
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
/// \brief This example demonstrates the geometry transforms in imstk
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    imstkNew<Scene> scene("GeometryTransforms");

    // Create object and add to scene
    auto SurfaceMesh = std::dynamic_pointer_cast<imstk::SurfaceMesh>(MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.obj"));
    auto sceneObj    = std::make_shared<SceneObject>("Dragon");
    sceneObj->setVisualGeometry(SurfaceMesh);
    scene->addSceneObject(sceneObj);

    auto surfaceMesh = sceneObj->getVisualGeometry();
    surfaceMesh->scale(5.0, Geometry::TransformType::ConcatenateToTransform);

    //  Plane
    imstkNew<Plane> planeGeom;
    planeGeom->scale(80.0, Geometry::TransformType::ConcatenateToTransform);
    planeGeom->translate(0.0, -20.0, 0.0, Geometry::TransformType::ConcatenateToTransform);
    planeGeom->rotate(Vec3d(0.0, 1.0, 0.0), PI_4, Geometry::TransformType::ConcatenateToTransform);

    imstkNew<RenderMaterial> planeMaterial;
    planeMaterial->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    planeMaterial->setPointSize(6.0);
    planeMaterial->setLineWidth(4.0);
    imstkNew<VisualModel> planeVisualModel;
    planeVisualModel->setGeometry(planeGeom);
    planeVisualModel->setRenderMaterial(planeMaterial);

    imstkNew<SceneObject> planeObj("Plane");
    planeObj->addVisualModel(planeVisualModel);
    scene->addSceneObject(planeObj);

    //  Cube
    imstkNew<OrientedBox> cubeGeom;
    cubeGeom->setExtents(5.0, 5.0, 5.0);
    cubeGeom->scale(0.5, Geometry::TransformType::ConcatenateToTransform);
    cubeGeom->rotate(Vec3d(1.0, 1.0, 0.0), PI_4, Geometry::TransformType::ApplyToData);
    cubeGeom->translate(Vec3d(0.0, 0.0, 10.0));

    auto                     materialCube = std::make_shared<RenderMaterial>();
    imstkNew<RenderMaterial> redMaterial;
    redMaterial->setColor(Color::Red);
    redMaterial->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    redMaterial->setPointSize(6.0);
    redMaterial->setLineWidth(4.0);
    imstkNew<VisualModel> cubeVisualModel;
    cubeVisualModel->setGeometry(cubeGeom);
    cubeVisualModel->setRenderMaterial(redMaterial);

    imstkNew<SceneObject> cubeObj("Cube");
    cubeObj->addVisualModel(cubeVisualModel);
    scene->addSceneObject(cubeObj);

    //  Cylinder
    imstkNew<Cylinder> cylinderGeom;
    cylinderGeom->setRadius(4.0);
    cylinderGeom->setLength(12.0);
    cylinderGeom->scale(0.4, Geometry::TransformType::ConcatenateToTransform);
    cylinderGeom->rotate(Vec3d(1.0, 1.0, 0), PI_2, Geometry::TransformType::ApplyToData);
    cylinderGeom->translate(Vec3d(0.0, 0.0, -10.0));

    imstkNew<VisualModel> cylVisualModel;
    cylVisualModel->setGeometry(cylinderGeom);
    cylVisualModel->setRenderMaterial(redMaterial);

    imstkNew<SceneObject> cylObj("Cylinder");
    cylObj->addVisualModel(cylVisualModel);
    scene->addSceneObject(cylObj);

    // Set Camera configuration
    scene->getActiveCamera()->setPosition(Vec3d(0.0, 30.0, 30.0));

    // Light
    imstkNew<DirectionalLight> light;
    light->setDirection(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight("light", light);

    // Run the simulation
    {
        // Setup a viewer to render in its own thread
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);

        // Rotate after every scene update
        connect<Event>(sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                surfaceMesh->rotate(Vec3d(1.0, 0.0, 0.0), PI * sceneManager->getDt());
                surfaceMesh->postModified();
        });

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControlEntity(driver);
        scene->addSceneObject(mouseAndKeyControls);

        driver->start();
    }

    return 0;
}
