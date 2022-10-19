/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkDirectionalLight.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLevelSetDeformableObject.h"
#include "imstkLevelSetModel.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkSurfaceMesh.h"
#include "imstkSurfaceMeshDistanceTransform.h"
#include "imstkVisualModel.h"
#include "imstkVolumeRenderMaterial.h"
#include "imstkVTKViewer.h"

#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolumeProperty.h>

using namespace imstk;

///
/// \brief Creates a shrinking level set dragon
///
std::shared_ptr<LevelSetDeformableObject>
makeLevelsetObj(const std::string& name)
{
    auto levelSetObj = std::make_shared<LevelSetDeformableObject>(name);

    // Setup the Geometry (read dragon mesh)
    auto dragonSurfMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/asianDragon/asianDragon.obj");
    //dragonSurfMesh->translate(position, Geometry::TransformType::ApplyToData);
    SurfaceMeshDistanceTransform computeSdf;
    computeSdf.setInputMesh(dragonSurfMesh);
    computeSdf.setDimensions(50, 50, 50);
    computeSdf.update();
    std::shared_ptr<ImageData> initLvlsetImage = computeSdf.getOutputImage();

    // Setup the Model
    auto dynamicalModel = std::make_shared<LevelSetModel>();
    dynamicalModel->getConfig()->m_sparseUpdate = false;
    dynamicalModel->getConfig()->m_dt = 0.003;
    dynamicalModel->getConfig()->m_constantVelocity = -1.0;
    dynamicalModel->setModelGeometry(initLvlsetImage);

    // Setup the VisualModel
    auto visualModel = levelSetObj->addComponent<VisualModel>();
    visualModel->setGeometry(initLvlsetImage);
    auto                             mat = std::make_shared<VolumeRenderMaterial>();
    vtkNew<vtkColorTransferFunction> color;
    color->AddRGBPoint(0.0, 0.0, 0.0, 1.0);
    color->AddRGBPoint(-0.01, 0.0, 0.0, 1.0);
    mat->getVolumeProperty()->SetColor(color);
    vtkNew<vtkPiecewiseFunction> opacity;
    opacity->AddPoint(0.0, 0.0);
    opacity->AddPoint(-0.01, 1.0);
    mat->getVolumeProperty()->SetScalarOpacity(opacity);
    mat->getVolumeProperty()->SetInterpolationTypeToLinear();
    mat->getVolumeProperty()->ShadeOn();
    visualModel->setRenderMaterial(mat);

    // Setup the Object
    levelSetObj->setPhysicsGeometry(initLvlsetImage);
    levelSetObj->setDynamicalModel(dynamicalModel);

    return levelSetObj;
}

///
/// \brief This example demonstrates evolution of a levelset
///
int
main()
{
    // Log to file and stdout
    Logger::startLogger();

    // Setup the scene
    auto scene = std::make_shared<Scene>("LevelSetExample");

    scene->addSceneObject(makeLevelsetObj("DragonLevelset"));

    // Light (white)
    auto whiteLight = std::make_shared<DirectionalLight>();
    whiteLight->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    whiteLight->setIntensity(1.0);
    scene->addLight("whitelight", whiteLight);

    // Adjust camera
    scene->getActiveCamera()->setPosition(0.0, 10.0, -10.0);

    // Run the simulation

    // Setup a viewer to render in its own thread
    auto viewer = std::make_shared<VTKViewer>();
    viewer->setActiveScene(scene);

    // Setup a scene manager to advance the scene in its own thread
    auto sceneManager = std::make_shared<SceneManager>();
    sceneManager->setActiveScene(scene);
    sceneManager->pause();

    auto driver = std::make_shared<SimulationManager>();
    driver->addModule(viewer);
    driver->addModule(sceneManager);
    driver->setDesiredDt(0.01);

    // Add default mouse and keyboard controls to the viewer
    std::shared_ptr<Entity> mouseAndKeyControls =
        SimulationUtils::createDefaultSceneControl(driver);
    scene->addSceneObject(mouseAndKeyControls);

    driver->start();

    return 0;
}
