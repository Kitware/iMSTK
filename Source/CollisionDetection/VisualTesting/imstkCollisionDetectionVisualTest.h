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

#pragma once

#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLogger.h"
#include "imstkMouseSceneControl.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkVTKTextStatusManager.h"
#include "imstkVTKViewer.h"

#include <gtest/gtest.h>

using namespace imstk;

template<typename T>
std::string
to_string_with_precision(const T a_value, const int n = 6)
{
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << a_value;
    return out.str();
}

class CollisionDetectionVisualTest : public testing::Test
{
public:
    ///
    /// \brief Run the simulation for given duration at given fixed timestep
    /// \param total time (seconds) to run the simulation
    /// \param fixed timestep (seconds)
    ///
    void runFor(const double duration, const double fixedTimestep = 0.001)
    {
        m_duration = duration;

        // Setup a viewer to render
        m_viewer = std::make_shared<VTKViewer>();
        m_viewer->setActiveScene(m_scene);
        m_viewer->setVtkLoggerMode(VTKViewer::VTKLoggerMode::MUTE);
        m_viewer->setDebugAxesLength(0.05, 0.05, 0.05);

        // Setup a scene manager to advance the scene
        m_sceneManager = std::make_shared<SceneManager>();
        m_sceneManager->setActiveScene(m_scene);

        m_driver = std::make_shared<SimulationManager>();
        m_driver->addModule(m_viewer);
        m_driver->addModule(m_sceneManager);

        // User/callee provided (should be called first)
        if (m_startingFunc != nullptr)
        {
            connect<Event>(m_driver, &SimulationManager::starting,
                m_startingFunc);
        }
        if (m_keyPressFunc != nullptr)
        {
            connect<KeyEvent>(m_viewer->getKeyboardDevice(), &KeyboardDeviceClient::keyPress,
                m_keyPressFunc);
        }
        if (m_postUpdateFunc)
        {
            connect<Event>(m_sceneManager, &SceneManager::postUpdate,
                m_postUpdateFunc);
        }
        if (m_preUpdateFunc != nullptr)
        {
            connect<Event>(m_sceneManager, &SceneManager::preUpdate,
                m_preUpdateFunc);
        }

        // For stopping the timer
        bool paused = false;
        connect<KeyEvent>(m_viewer->getKeyboardDevice(), &KeyboardDeviceClient::keyPress,
            [&](KeyEvent* e)
            {
                // If any key is pressed, stop the timer (stopping condition in postUpdate below)
                paused = true;

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
                if (paused)
                {
                    m_viewer->getTextStatusManager()->setCustomStatus("Paused at " +
                        to_string_with_precision(m_scene->getSceneTime(), 3) + 's');
                }
                else
                {
                    m_viewer->getTextStatusManager()->setCustomStatus(
                        to_string_with_precision(m_scene->getSceneTime(), 3) + 's');
                    if (m_scene->getSceneTime() > m_duration)
                    {
                        m_driver->requestStatus(ModuleDriverStopped);
                    }
                }
            });
        m_viewer->getTextStatusManager()->setCustomStatus("0.000s");

        // Add mouse and keyboard controls to the viewer
        {
            auto mouseControl = std::make_shared<MouseSceneControl>(m_viewer->getMouseDevice());
            mouseControl->setSceneManager(m_sceneManager);
            m_viewer->addControl(mouseControl);

            auto keyControl = std::make_shared<KeyboardSceneControl>(m_viewer->getKeyboardDevice());
            keyControl->setSceneManager(m_sceneManager);
            keyControl->setModuleDriver(m_driver);
            m_viewer->addControl(keyControl);
        }

        m_dt = fixedTimestep;
        m_driver->setDesiredDt(m_dt);
        m_driver->start();
    }

protected:
    double m_duration = 2.0;   ///< Duration to run the test
    double m_dt       = 0.001; ///< Fixed timestep

    std::function<void(Event*)>    m_startingFunc   = nullptr;
    std::function<void(KeyEvent*)> m_keyPressFunc   = nullptr;
    std::function<void(Event*)>    m_preUpdateFunc  = nullptr;
    std::function<void(Event*)>    m_postUpdateFunc = nullptr;

    std::shared_ptr<VTKViewer>         m_viewer       = nullptr;
    std::shared_ptr<SceneManager>      m_sceneManager = nullptr;
    std::shared_ptr<SimulationManager> m_driver       = nullptr;
    std::shared_ptr<Scene> m_scene = nullptr;
};