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

#include "imstkSceneManager.h"
#include "imstkScene.h"
#include "imstkLogger.h"
#include "imstkDeviceControl.h"

namespace imstk
{
SceneManager::SceneManager(std::string name) : m_activeScene(nullptr),
    m_mode(Mode::Simulation), m_prevCamName("default")
{
    // Set the preferred execution mode
    m_executionType = ExecutionType::ADAPTIVE;
}

void
SceneManager::setMode(Mode mode)
{
    if (m_mode == Mode::Simulation && mode == Mode::Debug)
    {
        // Store the previous camera name and switch to debug cam
        m_prevCamName =
            m_activeScene->getCameraName(m_activeScene->getActiveCamera());
        m_activeScene->setActiveCamera("debug");
    }
    else if (m_mode == Mode::Debug && mode == Mode::Simulation)
    {
        // Switch back
        m_activeScene->setActiveCamera(m_prevCamName);
    }
    m_mode = mode;
}

bool
SceneManager::containsScene(std::string name) const
{
    return (m_sceneMap.find(name) != m_sceneMap.end());
}

void
SceneManager::setActiveScene(std::string name)
{
    // Check if the requested scene exists
    if (!containsScene(name))
    {
        LOG(WARNING) << "Scene '" << name << "' not registered! Please register before setting active";
        return;
    }
    // Check if the scene is already active
    if (m_activeScene != nullptr && name == m_activeScene->getName())
    {
        LOG(INFO) << "Scene '" << name << "' is already active!";
        return;
    }

    auto newScene = this->getScene(name);

    m_activeScene = newScene;
}

void
SceneManager::setActiveScene(std::shared_ptr<Scene> scene)
{
    std::string name = scene->getName();

    // Check if the requested scene exists
    if (!containsScene(name))
    {
        addScene(scene);
    }
    setActiveScene(name);
}

bool
SceneManager::initModule()
{
    if (m_activeScene != nullptr)
    {
        return m_activeScene->initialize();
    }
    return true;
}

void
SceneManager::updateModule()
{
    // Advance the scene
    if (m_activeScene != nullptr)
    {
        // Process events given to this module
        this->doAllEvents();

        m_activeScene->advance(getDt());
    }
}

void
SceneManager::addScene(std::shared_ptr<Scene> scene)
{
    if (scene == nullptr)
    {
        LOG(WARNING) << "Tried to add null scene!";
        return;
    }
    m_sceneMap[scene->getName()] = scene;
}

void
SceneManager::removeScene(std::string name)
{
    if (containsScene(name))
    {
        std::shared_ptr<Scene> scene = m_sceneMap[name];
        if (m_activeScene == scene)
        {
            m_activeScene = nullptr;
        }
        m_sceneMap.erase(name);
    }
}
}