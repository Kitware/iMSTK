/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkVisualTestingUtils.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLogger.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkTestingUtils.h"
#include "imstkVTKTextStatusManager.h"
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
    m_viewer->setVtkLoggerMode(VTKViewer::VTKLoggerMode::MUTE);
    m_viewer->setDebugAxesLength(0.05, 0.05, 0.05);
    m_viewer->setWindowTitle(m_scene->getName());

    // Setup a scene manager to advance the scene
    m_sceneManager->setActiveScene(m_scene);

    m_driver->clearModules();
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
    connect<Event>(m_sceneManager, &SceneManager::postUpdate,
        [&](Event*)
        {
            if (m_timerPaused)
            {
                m_viewer->getTextStatusManager()->setCustomStatus("Paused at " +
                    to_string_with_precision(m_scene->getSceneTime(), 3) + 's');
            }
            else
            {
                m_viewer->getTextStatusManager()->setCustomStatus(
                    to_string_with_precision(m_scene->getSceneTime(), 3) + 's');
                if (m_duration != -1.0 && m_scene->getSceneTime() > m_duration)
                {
                    m_driver->requestStatus(ModuleDriverStopped);
                }
            }
        });
    m_viewer->getTextStatusManager()->setCustomStatus("0.000s");

    // Add mouse and keyboard controls to the viewer
    {
        auto mouseControl = std::make_shared<MouseSceneControl>();
        mouseControl->setDevice(m_viewer->getMouseDevice());
        mouseControl->setSceneManager(m_sceneManager);
        m_scene->addControl(mouseControl);

        auto keyControl = std::make_shared<KeyboardSceneControl>();
        keyControl->setUseTextStatus(false);
        keyControl->setDevice(m_viewer->getKeyboardDevice());
        keyControl->setSceneManager(m_sceneManager);
        keyControl->setModuleDriver(m_driver);
        m_scene->addControl(keyControl);
    }

    m_dt = fixedTimestep;
    m_driver->setDesiredDt(m_dt);
    m_driver->start();
}
