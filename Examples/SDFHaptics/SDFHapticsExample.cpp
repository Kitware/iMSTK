/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkAxesModel.h"
#include "imstkCamera.h"
#include "imstkCollidingObject.h"
#include "imstkDeviceManager.h"
#include "imstkDeviceManagerFactory.h"
#include "imstkDirectionalLight.h"
#include "imstkImplicitFunctionFiniteDifferenceFunctor.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkMeshIO.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkSurfaceMesh.h"
#include "imstkSurfaceMeshFlyingEdges.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief This example demonstrates manually applying point haptics
/// using an SDF
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    auto scene = std::make_shared<Scene>("SDFHaptics");
    scene->getActiveCamera()->setPosition(-2.3, 23.81, 45.65);
    scene->getActiveCamera()->setFocalPoint(9.41, 8.45, 5.76);

    auto                       bunnyObj = std::make_shared<CollidingObject>("Bunny");
    std::shared_ptr<ImageData> sdfImage = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/stanfordBunny/stanfordBunny_SDF.nii");
    auto                       sdf      = std::make_shared<SignedDistanceField>(sdfImage->cast(IMSTK_DOUBLE));
    {
        bunnyObj->setCollidingGeometry(sdf);

        SurfaceMeshFlyingEdges isoExtract;
        isoExtract.setInputImage(sdfImage);
        isoExtract.update();

        isoExtract.getOutputMesh()->flipNormals();
        bunnyObj->setVisualGeometry(isoExtract.getOutputMesh());
    }
    scene->addSceneObject(bunnyObj);

    auto axesObj   = std::make_shared<Entity>("Axes");
    auto axesModel = axesObj->addComponent<AxesModel>("AxesModel");
    scene->addSceneObject(axesObj);

    // Light
    auto light = std::make_shared<DirectionalLight>();
    light->setDirection(Vec3d(0.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight("light", light);

    // Setup default haptics manager
    std::shared_ptr<DeviceManager> hapticManager = DeviceManagerFactory::makeDeviceManager();
    std::shared_ptr<DeviceClient>  deviceClient  = hapticManager->makeDeviceClient();

    // Run the simulation
    {
        // Setup a viewer to render in its own thread
        auto viewer = std::make_shared<VTKViewer>();
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene in its own thread
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);

        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->addModule(hapticManager);

        ImplicitFunctionCentralGradient centralGrad;
        centralGrad.setFunction(sdf);
        centralGrad.setDx(sdf->getImage()->getSpacing());
        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
            {
                const Vec3d pos = deviceClient->getPosition() * 100.0 + Vec3d(10.0, 0.1, 10.0);

                axesModel->setPosition(pos);
                axesModel->setOrientation(deviceClient->getOrientation());

                double dx = sdf->getFunctionValue(pos);
                if (dx < 0.0)
                {
                    const Vec3d g = centralGrad(pos);
                    deviceClient->setForce(-g.normalized() * dx * 4.0);
                }
            });

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
        scene->addSceneObject(mouseAndKeyControls);

        driver->start();
    }

    return 0;
}