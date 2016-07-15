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

namespace imstk {
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

std::shared_ptr<Scene>
SimulationManager::getScene(std::string sceneName) const
{
    if (!this->isSceneRegistered(sceneName))
    {
        LOG(WARNING) << "No scene named '" << sceneName
                     << "' was registered in this simulation";
        return nullptr;
    }

    return m_sceneManagerMap.at(sceneName)->getScene();
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
    int id                   = m_sceneManagerMap.size() + 1;
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
SimulationManager::isDeviceServerRegistered(std::string serverName) const
{
    return m_deviceServerMap.find(serverName) != m_deviceServerMap.end();
}

std::shared_ptr<VRPNDeviceServer>
SimulationManager::getDeviceServer(std::string serverName) const
{
    if (!this->isDeviceServerRegistered(serverName))
    {
        LOG(WARNING) << "No device server at '" << serverName
                     << "' was registered in this simulation";
        return nullptr;
    }

    return m_deviceServerMap.at(serverName);
}
void
SimulationManager::addDeviceServer(std::shared_ptr<VRPNDeviceServer> newServer)
{
    std::string newServerName = newServer->getName();

    if (this->isDeviceServerRegistered(newServerName))
    {
        LOG(WARNING) << "Can not add device server: '" << newServerName
                     << "' is already registered in this simulation\n"
                     << "Set this server address to a unique ip:port first";
        return;
    }

    m_deviceServerMap[newServerName] = newServer;
    LOG(INFO) << "Device server added: " << newServerName;
}

void
SimulationManager::removeDeviceServer(std::string serverName)
{
    if (!this->isDeviceServerRegistered(serverName))
    {
        LOG(WARNING) << "No device server at '" << serverName
                     << "' was registered in this simulation";
        return;
    }

    m_deviceServerMap.erase(serverName);
    LOG(INFO) << "Device server removed: " << serverName;
}

bool
SimulationManager::isDeviceClientRegistered(std::string deviceClientName) const
{
    return m_deviceClientMap.find(deviceClientName) != m_deviceClientMap.end();
}

std::shared_ptr<DeviceClient>
SimulationManager::getDeviceClient(std::string deviceClientName) const
{
    if (!this->isDeviceClientRegistered(deviceClientName))
    {
        LOG(WARNING) << "No device client named '" << deviceClientName
                     << "' was registered in this simulation";
        return nullptr;
    }

    return m_deviceClientMap.at(deviceClientName);
}

void
SimulationManager::addDeviceClient(std::shared_ptr<DeviceClient> newDeviceClient)
{
    std::string newDeviceClientName = newDeviceClient->getName();

    if (this->isDeviceClientRegistered(newDeviceClientName))
    {
        LOG(WARNING) << "Can not add device client: '" << newDeviceClientName
                     << "' is already registered in this simulation\n"
                     << "Set this device name to a unique name first";
        return;
    }

    m_deviceClientMap[newDeviceClientName] = newDeviceClient;
    LOG(INFO) << "Device client added: " << newDeviceClientName;
}

void
SimulationManager::removeDeviceClient(std::string deviceClientName)
{
    if (!this->isDeviceClientRegistered(deviceClientName))
    {
        LOG(WARNING) << "No device client named '" << deviceClientName
                     << "' was registered in this simulation";
        return;
    }

    m_deviceClientMap.erase(deviceClientName);
    LOG(INFO) << "Device client removed: " << deviceClientName;
}

std::shared_ptr<Viewer>
SimulationManager::getViewer() const
{
    return m_viewer;
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

    std::shared_ptr<Scene> newScene = this->getScene(newSceneName);
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
        m_viewer->setRenderingMode(Renderer::Mode::DEBUG);
        m_currentSceneName = newSceneName;
        return;
    }

    // If rendering and simulation active:
    // render scene in simulation mode, and update simulation
    m_viewer->setRenderingMode(Renderer::Mode::SIMULATION);

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
        m_viewer->setRenderingMode(Renderer::Mode::SIMULATION);

        // Start device servers
        for(const auto& pair : m_deviceServerMap)
        {
            this->startModuleInNewThread(pair.second);
        }

        // Start device clients
        for(const auto& pair : m_deviceClientMap)
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
        m_viewer->setRenderingMode(Renderer::Mode::DEBUG);
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

    // Run device servers
    for(const auto& pair : m_deviceServerMap)
    {
        (pair.second)->run();
    }

    // Run device clients
    for(const auto& pair : m_deviceClientMap)
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

    // Pause device clients
    for(const auto& pair : m_deviceClientMap)
    {
        (pair.second)->pause();
    }

    // Pause device servers
    for(const auto& pair : m_deviceServerMap)
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
    m_viewer->setRenderingMode(Renderer::Mode::DEBUG);

    // End device clients
    for(const auto& pair : m_deviceClientMap)
    {
        (pair.second)->end();
        m_threadMap.at(pair.first).join();
    }

    // Pause device servers
    for(const auto& pair : m_deviceServerMap)
    {
        (pair.second)->end();
        m_threadMap.at(pair.first).join();
    }

    // End all scenes
    for (auto pair : m_sceneManagerMap)
    {
        std::string  sceneName   = pair.first;
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
}
