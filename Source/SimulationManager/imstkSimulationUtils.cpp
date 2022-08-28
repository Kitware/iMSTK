/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkSimulationUtils.h"
#include "imstkFpsTxtCounter.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkPerformanceGraph.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkViewer.h"

namespace imstk
{
namespace SimulationUtils
{
std::shared_ptr<Entity>
createDefaultSceneControlEntity(
    std::shared_ptr<SimulationManager> driver)
{
    std::shared_ptr<SceneManager> sceneManager = nullptr;
    std::shared_ptr<Viewer>       viewer       = nullptr;
    for (auto driverModule : driver->getModules())
    {
        if (auto viewerModule = std::dynamic_pointer_cast<Viewer>(driverModule))
        {
            viewer = viewerModule;
        }
        else if (auto sceneManagerModule = std::dynamic_pointer_cast<SceneManager>(driverModule))
        {
            sceneManager = sceneManagerModule;
        }
    }
    CHECK(sceneManager != nullptr) << "The SimulationManager has no SceneManager";
    CHECK(viewer != nullptr) << "The SimulationManager has no Viewer";

    // Setup a text to render the state of the simulation (if paused or not)
    auto statusText = std::make_shared<SceneControlText>();
    statusText->setSceneManager(sceneManager);

    // Setup a default key control scheme (commonly used in examples)
    auto keyControl = std::make_shared<KeyboardSceneControl>();
    keyControl->setDevice(viewer->getKeyboardDevice());
    keyControl->setSceneManager(sceneManager);
    keyControl->setModuleDriver(driver);
    keyControl->setSceneControlText(statusText);

    // Setup a default mouse control scheme (commonly used in examples)
    auto mouseControl = std::make_shared<MouseSceneControl>();
    mouseControl->setDevice(viewer->getMouseDevice());
    mouseControl->setSceneManager(sceneManager);

    // Setup an fps counter (toggled on info level of viewer)
    auto fpsCounter = std::make_shared<FpsTxtCounter>();
    fpsCounter->setSceneManager(sceneManager);
    fpsCounter->setViewer(viewer);

    // Setup a task node performance graph (toggled on info level of viewer)
    auto perfGraph = std::make_shared<PerformanceGraph>();
    perfGraph->setSceneManager(sceneManager);
    perfGraph->setViewer(viewer);

    return Entity::createEntity(
        keyControl,
        statusText,
        mouseControl,
        fpsCounter,
        perfGraph);
}
} // namespace SimulationUtils
} // namespace imstk