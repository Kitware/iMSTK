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

#include "imstkSimulationManager.h"

#include <string>

namespace imstk {
const
SimulationStatus& SimulationManager::getStatus() const
{
    return m_status;
}

bool
SimulationManager::isSceneRegistered(std::string sceneName)
{
    return !(m_sceneMap.find(sceneName) == m_sceneMap.end());
}

std::shared_ptr<Scene>
SimulationManager::getScene(std::string sceneName)
{
    if (!this->isSceneRegistered(sceneName))
    {
        std::cerr << "No scene named '" << sceneName
                  << "' was registered in this simulation." << std::endl;
        return nullptr;
    }

    return m_sceneMap.at(sceneName);
}

std::shared_ptr<Scene>
SimulationManager::createNewScene(std::string newSceneName)
{
    if (this->isSceneRegistered(newSceneName))
    {
        std::cerr << "Can not create new scene: '" << newSceneName
                  << "' is already registered in this simulation." << std::endl
                  << "You can create a new scene using an unique name." << std::endl;
        return nullptr;
    }

    m_sceneMap[newSceneName] = std::make_shared<Scene>(newSceneName);
    std::cout << "New scene added: " << newSceneName << std::endl;
    return m_sceneMap.at(newSceneName);
}

std::shared_ptr<Scene>
SimulationManager::createNewScene()
{
    int id                   = m_sceneMap.size() + 1;
    std::string newSceneName = "Scene_" + std::to_string(id);

    return this->createNewScene(newSceneName);
}

void
SimulationManager::addScene(std::shared_ptr<Scene>newScene)
{
    std::string newSceneName = newScene->getName();

    if (this->isSceneRegistered(newSceneName))
    {
        std::cerr << "Can not add scene: '" << newSceneName
                  << "' is already registered in this simulation." << std::endl
                  << "Set this scene name to a unique name first." << std::endl;
        return;
    }

    m_sceneMap[newSceneName] = newScene;
    std::cout << "Scene added: " << newSceneName << std::endl;
}

void
SimulationManager::removeScene(std::string sceneName)
{
    if (!this->isSceneRegistered(sceneName))
    {
        std::cerr << "No scene named '" << sceneName
                  << "' was registered in this simulation." << std::endl;
        return;
    }

    m_sceneMap.erase(sceneName);
    std::cout << "Scene removed: " << sceneName << std::endl;
}

void
SimulationManager::startSimulation(std::string sceneName)
{
    std::cout << "Starting simulation." << std::endl;

    if (m_status != SimulationStatus::INACTIVE)
    {
        std::cerr << "Simulation already active." << std::endl;
        return;
    }

    std::shared_ptr<Scene> startingScene = this->getScene(sceneName);

    if (!startingScene)
    {
        std::cerr << "Simulation canceled." << std::endl;
        return;
    }

    if (startingScene->getStatus() != ModuleStatus::INACTIVE)
    {
        std::cerr << "Scene '" << sceneName << "' is already active." << std::endl
                  << "Simulation canceled." << std::endl;
        return;
    }

    // Start scene
    this->startModuleInNewThread(startingScene);
    m_currentSceneName = sceneName;

    // Update simulation status
    m_status = SimulationStatus::RUNNING;
}

void
SimulationManager::switchScene(std::string newSceneName, bool unloadCurrentScene)
{
    std::cout << "Switching scene." << std::endl;

    if ((m_status != SimulationStatus::RUNNING) &&
        (m_status != SimulationStatus::PAUSED))
    {
        std::cerr << "Simulation not active, can not switch scenes." << std::endl;
        return;
    }

    if (newSceneName == m_currentSceneName)
    {
        std::cerr << "Scene '" << newSceneName << "' is already running." << std::endl;
        return;
    }

    std::shared_ptr<Scene> newScene = this->getScene(newSceneName);

    if (!newScene)
    {
        std::cerr << "Can not switch scenes." << std::endl;
        return;
    }

    if (unloadCurrentScene)
    {
        // Stop current scene
        std::cout << "Unloading '" << m_currentSceneName << "'." << std::endl;
        m_sceneMap.at(m_currentSceneName)->end();
        m_threadMap.at(m_currentSceneName).join();
    }
    else
    {
        // Pause current scene
        m_sceneMap.at(m_currentSceneName)->pause();
    }

    if (newScene->getStatus() == ModuleStatus::INACTIVE)
    {
        // Start new scene
        this->startModuleInNewThread(newScene);
    }
    else if (newScene->getStatus() == ModuleStatus::PAUSED)
    {
        // Run new scene
        newScene->run();
    }
    m_currentSceneName = newSceneName;
}

void
SimulationManager::runSimulation()
{
    std::cout << "Running simulation." << std::endl;

    if (m_status != SimulationStatus::PAUSED)
    {
        std::cerr << "Simulation not paused, can not run." << std::endl;
        return;
    }

    m_sceneMap.at(m_currentSceneName)->run();

    m_status = SimulationStatus::RUNNING;
}

void
SimulationManager::pauseSimulation()
{
    std::cout << "Pausing simulation." << std::endl;

    if (m_status != SimulationStatus::RUNNING)
    {
        std::cerr << "Simulation not running, can not pause." << std::endl;
        return;
    }

    // Pause scene
    m_sceneMap.at(m_currentSceneName)->pause();

    // Update simulation status
    m_status = SimulationStatus::PAUSED;
}

void
SimulationManager::endSimulation()
{
    std::cout << "Ending simulation." << std::endl;

    if ((m_status != SimulationStatus::RUNNING) &&
        (m_status != SimulationStatus::PAUSED))
    {
        std::cerr << "Simulation already terminated." << std::endl;
        return;
    }

    // End all scenes
    for (auto pair : m_sceneMap)
    {
        std::string  sceneName   = pair.first;
        ModuleStatus sceneStatus = m_sceneMap.at(sceneName)->getStatus();

        if (sceneStatus != ModuleStatus::INACTIVE)
        {
            m_sceneMap.at(sceneName)->end();
            m_threadMap.at(sceneName).join();
        }
    }

    // Update simulation status
    m_status = SimulationStatus::INACTIVE;
}

void
SimulationManager::startModuleInNewThread(std::shared_ptr<Module>module)
{
    m_threadMap[module->getName()] = std::thread([module] { module->start(); });
}
}
