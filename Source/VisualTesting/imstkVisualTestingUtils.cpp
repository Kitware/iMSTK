/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkVisualTestingUtils.h"
#include "imstkEntity.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLogger.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkTestingUtils.h"
#include "imstkTextVisualModel.h"
#include "imstkVTKRenderer.h"
#include "imstkVTKViewer.h"

using namespace imstk;

void
VisualTest::SetUp()
{
#ifndef iMSTK_SYNCHRONOUS_LOGGING
    if (m_useStdOut)
    {
        Logger::getInstance().addStdoutSink();
    }
#endif
    // Constructed early so user can subscribe calls to them
    m_driver       = std::make_shared<SimulationManager>();
    m_sceneManager = std::make_shared<SceneManager>();
    m_viewer       = std::make_shared<VTKViewer>();
    m_viewer->setVtkLoggerMode(VTKViewer::VTKLoggerMode::MUTE);
    m_viewer->setDebugAxesLength(0.05, 0.05, 0.05);
}

void
VisualTest::TearDown()
{
#ifndef iMSTK_SYNCHRONOUS_LOGGING
    Logger::getInstance().destroy();
#endif
}

void
VisualTest::runFor(const double duration, const double fixedTimestep)
{
    m_duration = duration;

    // Setup a viewer to render
    m_viewer->setActiveScene(m_scene);
    m_viewer->setWindowTitle(m_scene->getName());

    // Setup a scene manager to advance the scene
    m_sceneManager->setActiveScene(m_scene);

    m_driver->clearModules();
    // All tests are completely deterministic, ensure completely fixed timestep
    m_driver->setUseRemainderTimeDivide(false);
    m_driver->requestStatus(ModuleDriverRunning);
    m_driver->addModule(m_viewer);
    m_driver->addModule(m_sceneManager);

    // For stopping the timer
    connect<KeyEvent>(m_viewer->getKeyboardDevice(), &KeyboardDeviceClient::keyPress,
        [&](KeyEvent* e)
        {
            // If any key is pressed, stop the timer (stopping condition in postUpdate below)
            m_timerPaused = true;

            // If key advance is on
            if (e->m_key == '1')
            {
                m_sceneManager->setDt(0.05);
                m_sceneManager->postEvent(Event(SceneManager::preUpdate()));
                m_scene->advance(0.05);
                m_sceneManager->postEvent(Event(SceneManager::postUpdate()));
            }
            else if (e->m_key == '2')
            {
                m_sceneManager->setDt(0.01);
                m_sceneManager->postEvent(Event(SceneManager::preUpdate()));
                m_scene->advance(0.01);
                m_sceneManager->postEvent(Event(SceneManager::postUpdate()));
            }
            else if (e->m_key == '3')
            {
                m_sceneManager->setDt(0.001);
                m_sceneManager->postEvent(Event(SceneManager::preUpdate()));
                m_scene->advance(0.001);
                m_sceneManager->postEvent(Event(SceneManager::postUpdate()));
            }
        });

    // Setup a default key control scheme (commonly used in examples)
    auto debugEntity = std::make_shared<Entity>();
    auto keyControl  = debugEntity->addComponent<KeyboardSceneControl>();
    keyControl->setDevice(m_viewer->getKeyboardDevice());
    keyControl->setSceneManager(m_sceneManager);
    keyControl->setModuleDriver(m_driver);

    // Setup a default mouse control scheme (commonly used in examples)
    auto mouseControl = debugEntity->addComponent<MouseSceneControl>();
    mouseControl->setDevice(m_viewer->getMouseDevice());
    mouseControl->setSceneManager(m_sceneManager);

    // Add extra text object to display time and paused status of the test
    auto testStatusTxtModel = debugEntity->addComponent<TextVisualModel>("TestStatusText");
    testStatusTxtModel->setFontSize(30.0);
    testStatusTxtModel->setPosition(TextVisualModel::DisplayPosition::UpperLeft);
    testStatusTxtModel->setText("0.000s");

    m_scene->addSceneObject(debugEntity);

    connect<Event>(m_sceneManager, &SceneManager::postUpdate,
        [&](Event*)
        {
            if (m_timerPaused)
            {
                testStatusTxtModel->setText("Paused at " +
                    to_string_with_precision(m_scene->getSceneTime(), 3) + 's');
            }
            else
            {
                testStatusTxtModel->setText(
                    to_string_with_precision(m_scene->getSceneTime(), 3) + 's');
                if (m_duration != -1.0 && m_scene->getSceneTime() > m_duration)
                {
                    m_driver->requestStatus(ModuleDriverStopped);
                }
            }
        });

    m_dt = fixedTimestep;
    m_driver->setDesiredDt(m_dt);
    m_driver->start();
}
