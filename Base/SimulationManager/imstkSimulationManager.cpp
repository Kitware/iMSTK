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

#include "g3log/g3log.hpp"

namespace imstk
{
const
SimulationStatus& SimulationManager::getStatus() const
{
    return m_status;
}

bool
SimulationManager::isSceneRegistered(std::string sceneName) const
{
    return m_sceneManagerMap.find(sceneName) != m_sceneManagerMap.end();
}

std::shared_ptr<SceneManager>
SimulationManager::getSceneManager(std::string sceneName) const
{
    if (!this->isSceneRegistered(sceneName))
    {
        LOG(WARNING) << "No scene named '" << sceneName
                     << "' was registered in this simulation";
        return nullptr;
    }

    return m_sceneManagerMap.at(sceneName);
}

std::shared_ptr<Scene>
SimulationManager::getScene(std::string sceneName) const
{
    auto sceneManager = this->getSceneManager(sceneName);
    return sceneManager ? sceneManager->getScene() : nullptr;
}

std::shared_ptr<Scene>
SimulationManager::getCurrentScene() const
{
    return this->getScene(m_currentSceneName);
}

std::shared_ptr<Scene>
SimulationManager::createNewScene(std::string newSceneName)
{
    if (this->isSceneRegistered(newSceneName))
    {
        LOG(WARNING) << "Can not create new scene: '" << newSceneName
                     << "' is already registered in this simulation\n"
                     << "You can create a new scene using an unique name";
        return nullptr;
    }

    auto newScene = std::make_shared<Scene>(newSceneName);
    m_sceneManagerMap[newSceneName] = std::make_shared<SceneManager>(newScene);
    LOG(INFO) << "New scene added: " << newSceneName;
    return newScene;
}

std::shared_ptr<Scene>
SimulationManager::createNewScene()
{
    int id = (int)m_sceneManagerMap.size() + 1;
    std::string newSceneName = "Scene_" + std::to_string(id);

    return this->createNewScene(newSceneName);
}

void
SimulationManager::addScene(std::shared_ptr<Scene> newScene)
{
    std::string newSceneName = newScene->getName();

    if (this->isSceneRegistered(newSceneName))
    {
        LOG(WARNING) << "Can not add scene: '" << newSceneName
                     << "' is already registered in this simulation\n"
                     << "Set this scene name to a unique name first";
        return;
    }

    m_sceneManagerMap[newSceneName] = std::make_shared<SceneManager>(newScene);
    LOG(INFO) << "Scene added: " << newSceneName;
}

void
SimulationManager::removeScene(std::string sceneName)
{
    if (!this->isSceneRegistered(sceneName))
    {
        LOG(WARNING) << "No scene named '" << sceneName
                     << "' was registered in this simulation";
        return;
    }

    m_sceneManagerMap.erase(sceneName);
    LOG(INFO) << "Scene removed: " << sceneName;
}

bool
SimulationManager::isModuleRegistered(std::string moduleName) const
{
    return m_modulesMap.find(moduleName) != m_modulesMap.end();
}

std::shared_ptr<Module>
SimulationManager::getModule(std::string moduleName) const
{
    if (!this->isModuleRegistered(moduleName))
    {
        LOG(WARNING) << "No module named '" << moduleName
                     << "' was registered in this simulation";
        return nullptr;
    }

    return m_modulesMap.at(moduleName);
}

void
SimulationManager::addModule(std::shared_ptr<Module> newModule)
{
    std::string newModuleName = newModule->getName();

    if (this->isModuleRegistered(newModuleName))
    {
        LOG(WARNING) << "Can not addmodule: '" << newModuleName
                     << "' is already registered in this simulation\n";
        return;
    }

    m_modulesMap[newModuleName] = newModule;
    LOG(INFO) << "Module added: " << newModuleName;
}

void
SimulationManager::removeModule(std::string moduleName)
{
    if (!this->isModuleRegistered(moduleName))
    {
        LOG(WARNING) << "No module named '" << moduleName
                     << "' was registered in this simulation";
        return;
    }

    m_modulesMap.erase(moduleName);
    LOG(INFO) << "Module removed: " << moduleName;
}

std::shared_ptr<VTKViewer>
SimulationManager::getViewer() const
{
    return m_viewer;
}

void
SimulationManager::setCurrentScene(std::shared_ptr<Scene> scene, bool unloadCurrentScene)
{
    setCurrentScene(scene->getName(), unloadCurrentScene);
}

void
SimulationManager::setCurrentScene(std::string newSceneName, bool unloadCurrentScene)
{
    LOG(INFO) << "Setting current scene";

    if (newSceneName == m_currentSceneName)
    {
        LOG(WARNING) << "Scene '" << newSceneName << "' is already current";
        return;
    }

    auto newScene = this->getScene(newSceneName);
    if (!newScene)
    {
        LOG(WARNING) << "Can not find scene";
        return;
    }

    // Update viewer scene
    m_viewer->setCurrentScene(newScene);

    // If not yet rendering: update current scene and return
    if(!m_viewer->isRendering())
    {
        m_currentSceneName = newSceneName;
        return;
    }

    // If rendering and simulation not active:
    // render scene in debug, update current scene, and return
    if (m_status == SimulationStatus::INACTIVE)
    {
        m_viewer->setRenderingMode(VTKRenderer::Mode::DEBUG);
        m_currentSceneName = newSceneName;
        return;
    }

    // If rendering and simulation active:
    // render scene in simulation mode, and update simulation
    m_viewer->setRenderingMode(VTKRenderer::Mode::SIMULATION);

    // Stop/Pause running scene
    auto oldSceneManager = m_sceneManagerMap.at(m_currentSceneName);
    if (unloadCurrentScene)
    {
        LOG(INFO) << "Unloading '" << m_currentSceneName << "'";
        oldSceneManager->end();
        m_threadMap.at(m_currentSceneName).join();
    }
    else
    {
        oldSceneManager->pause();
    }

    // Start/Run new scene
    auto newSceneManager = m_sceneManagerMap.at(newSceneName);
    if (newSceneManager->getStatus() == ModuleStatus::INACTIVE)
    {
        this->startModuleInNewThread(newSceneManager);
    }
    else if (newSceneManager->getStatus() == ModuleStatus::PAUSED)
    {
        newSceneManager->run();
    }
    m_currentSceneName = newSceneName;
}

void
SimulationManager::startSimulation(bool debug)
{
    this->getCurrentScene()->initialize();
    if (m_status != SimulationStatus::INACTIVE)
    {
        LOG(WARNING) << "Simulation already active";
        return;
    }

    std::shared_ptr<Scene> startingScene = this->getCurrentScene();
    if (!startingScene)
    {
        LOG(WARNING) << "Simulation canceled";
        return;
    }

    auto startingSceneManager = m_sceneManagerMap.at(m_currentSceneName);
    if (startingSceneManager->getStatus() != ModuleStatus::INACTIVE)
    {
        LOG(WARNING) << "Scene '" << m_currentSceneName << "' is already active";
        return;
    }

    // Simulation
    if( !debug )
    {
        LOG(INFO) << "Starting simulation";
        m_viewer->setRenderingMode(VTKRenderer::Mode::SIMULATION);

        // Start modules
        for(const auto& pair : m_modulesMap)
        {
            this->startModuleInNewThread(pair.second);
        }

        // Start scene
        this->startModuleInNewThread(startingSceneManager);

        m_status = SimulationStatus::RUNNING;
    }
    // Debug
    else
    {
        m_viewer->setRenderingMode(VTKRenderer::Mode::DEBUG);
    }

    // Start Rendering
    if( !m_viewer->isRendering() )
    {
        LOG(INFO) << "Starting viewer";
        m_viewer->startRenderingLoop();
        LOG(INFO) << "Closing viewer";

        // End simulation if active when loop exits
        if (m_status != SimulationStatus::INACTIVE)
        {
            this->endSimulation();
        }
    }
}

void
SimulationManager::runSimulation()
{
    LOG(INFO) << "Running simulation";

    if (m_status != SimulationStatus::PAUSED)
    {
        LOG(WARNING) << "Simulation not paused, can not run";
        return;
    }

    // Run scene
    m_sceneManagerMap.at(m_currentSceneName)->run();

    // Run modules
    for(const auto& pair : m_modulesMap)
    {
        (pair.second)->run();
    }

    // Update simulation status
    m_status = SimulationStatus::RUNNING;
}

void
SimulationManager::pauseSimulation()
{
    LOG(INFO) << "Pausing simulation";

    if (m_status != SimulationStatus::RUNNING)
    {
        LOG(WARNING) << "Simulation not running, can not pause";
        return;
    }

    // Pause scene
    m_sceneManagerMap.at(m_currentSceneName)->pause();

    // Pause modules
    for(const auto& pair : m_modulesMap)
    {
        (pair.second)->pause();
    }

    // Update simulation status
    m_status = SimulationStatus::PAUSED;
}

void
SimulationManager::endSimulation()
{
    LOG(INFO) << "Ending simulation";

    if ((m_status != SimulationStatus::RUNNING) &&
        (m_status != SimulationStatus::PAUSED))
    {
        LOG(WARNING) << "Simulation already terminated";
        return;
    }

    // Update Renderer
    m_viewer->setRenderingMode(VTKRenderer::Mode::DEBUG);

    // End modules
    for(const auto& pair : m_modulesMap)
    {
        (pair.second)->end();
        m_threadMap.at(pair.first).join();
    }

    // End all scenes
    for (auto pair : m_sceneManagerMap)
    {
        std::string sceneName   = pair.first;
        ModuleStatus sceneStatus = pair.second->getStatus();

        if (sceneStatus != ModuleStatus::INACTIVE)
        {
            m_sceneManagerMap.at(sceneName)->end();
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
} // imstk
