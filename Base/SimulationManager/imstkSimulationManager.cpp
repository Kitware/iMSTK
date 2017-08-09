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
const SimulationStatus&
SimulationManager::getStatus() const
{
    return m_status;
}

bool
SimulationManager::isSceneRegistered(const std::string& sceneName) const
{
    return m_sceneManagerMap.find(sceneName) != m_sceneManagerMap.end();
}

std::shared_ptr<SceneManager>
SimulationManager::getSceneManager(const std::string& sceneName) const
{
    if (!this->isSceneRegistered(sceneName))
    {
        LOG(WARNING) << "No scene named '" << sceneName
                     << "' was registered in this simulation";
        return nullptr;
    }

    return m_sceneManagerMap.at(sceneName);
}

std::shared_ptr<imstk::SceneManager>
SimulationManager::getSceneManager(std::shared_ptr<Scene> scene) const
{
    if (!scene)
    {
        LOG(WARNING) << "SimulationManager::getSceneManager - Scene supplied is not valid!";
    }
    return this->getSceneManager(scene->getName());
}

std::shared_ptr<Scene>
SimulationManager::getScene(const std::string& sceneName) const
{
    auto sceneManager = this->getSceneManager(sceneName);
    return sceneManager ? sceneManager->getScene() : nullptr;
}

std::shared_ptr<Scene>
SimulationManager::getActiveScene() const
{
    return this->getScene(m_activeSceneName);
}

std::shared_ptr<Scene>
SimulationManager::createNewScene(const std::string& newSceneName)
{
    if (this->isSceneRegistered(newSceneName))
    {
        LOG(WARNING) << "Can not create new scene: '" << newSceneName
                     << "' is already registered in this simulation\n"
                     << "You can create a new scene using an unique name";
        return nullptr;
    }

    auto newScene = std::make_shared<Scene>(newSceneName);
    m_sceneManagerMap[newScene->getName()] = std::make_shared<SceneManager>(newScene);
    LOG(INFO) << "New scene added: " << newScene->getName();
    return newScene;
}

std::shared_ptr<Scene>
SimulationManager::createNewScene(std::string&& newSceneName)
{
    if (this->isSceneRegistered(newSceneName))
    {
        LOG(WARNING) << "Can not create new scene: '" << newSceneName
            << "' is already registered in this simulation\n"
            << "You can create a new scene using an unique name";
        return nullptr;
    }

    auto newScene = std::make_shared<Scene>(std::move(newSceneName));
    m_sceneManagerMap[newScene->getName()] = std::make_shared<SceneManager>(newScene);
    LOG(INFO) << "New scene added: " << newScene->getName();
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
SimulationManager::removeScene(const std::string& sceneName)
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
SimulationManager::isModuleRegistered(const std::string& moduleName) const
{
    return m_modulesMap.find(moduleName) != m_modulesMap.end();
}

std::shared_ptr<Module>
SimulationManager::getModule(const std::string& moduleName) const
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
        LOG(WARNING) << "Can not add module: '" << newModuleName
                     << "' is already registered in this simulation\n";
        return;
    }

    m_modulesMap[newModuleName] = newModule;
    LOG(INFO) << "Module added: " << newModuleName;
}

void
SimulationManager::removeModule(const std::string& moduleName)
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
SimulationManager::setActiveScene(std::shared_ptr<Scene> scene,
                                  const bool unloadCurrentScene /*= false*/)
{
    this->setActiveScene(scene->getName(), unloadCurrentScene);
}

void
SimulationManager::setActiveScene(const std::string& newSceneName,
                                  const bool unloadCurrentScene /*= false*/)
{
    LOG(INFO) << "SimulationManager::setActiveScene - Setting " << newSceneName << " as active";

    if (newSceneName == m_activeSceneName)
    {
        LOG(WARNING) << "\tScene '" << newSceneName << "' is already active";
        return;
    }

    auto newScene = this->getScene(newSceneName);
    if (!newScene)
    {
        LOG(WARNING) << "\tCan not find scene";
        return;
    }

    // Update viewer scene
    m_viewer->setActiveScene(newScene);

    // If not yet rendering: update current scene and return
    if(!m_viewer->isRendering())
    {
        m_activeSceneName = newSceneName;
        return;
    }

    // If rendering and simulation not active:
    // render scene in debug, update current scene, and return
    if (m_status == SimulationStatus::INACTIVE)
    {
        m_viewer->setRenderingMode(VTKRenderer::Mode::DEBUG);
        m_activeSceneName = newSceneName;
        return;
    }

    // If rendering and simulation active:
    // render scene in simulation mode, and update simulation
    m_viewer->setRenderingMode(VTKRenderer::Mode::SIMULATION);

    // Stop/Pause running scene
    auto oldSceneManager = m_sceneManagerMap.at(m_activeSceneName);
    if (unloadCurrentScene)
    {
        LOG(INFO) << "\tUnloading '" << m_activeSceneName << "'";
        oldSceneManager->end();
        m_threadMap.at(m_activeSceneName).join();
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
    m_activeSceneName = newSceneName;
}


void
SimulationManager::launchSimulation()
{
    if (m_status == SimulationStatus::RUNNING)
    {
        LOG(WARNING) << "SimulationManager::launchSimulation() - Simulation already running!";
        return;
    }

    if (!this->getActiveScene())
    {
        LOG(WARNING) << "SimulationManager::launchSimulation - No valid active scene! Simulation canceled";
        return;
    }

    if (!this->getActiveScene()->isInitialized())
    {
        if (!this->getActiveScene()->initialize())
        {
            LOG(WARNING) << "SimulationManager::startSimulation - Unable to initialize the active scene - "
                         << this->getActiveScene()->getName() << std::endl;
            return;
        }
    }

    // Start modules
    for (const auto& pair : m_modulesMap)
    {
        this->startModuleInNewThread(pair.second);
    }

    // Start scene
    this->startModuleInNewThread(m_sceneManagerMap.at(m_activeSceneName));

    m_status = SimulationStatus::RUNNING;
}

void
SimulationManager::startSimulation(const bool startSimulationPaused /*= true*/,
                                   const bool viewerInDebugMode /*= false*/)
{
    if (!this->getActiveScene())
    {
        LOG(WARNING) << "SimulationManager::startSimulation - No valid active scene! Simulation canceled";
        return;
    }

    if (!this->getActiveScene()->isInitialized())
    {
        if (!this->getActiveScene()->initialize())
        {
            LOG(WARNING) << "SimulationManager::startSimulation - Unable to initialize the active scene - "
                         << this->getActiveScene()->getName() << std::endl;
            return;
        }
    }

    if (m_status != SimulationStatus::INACTIVE)
    {
        LOG(WARNING) << "Simulation already active";
        return;
    }

    auto startingSceneManager = m_sceneManagerMap.at(m_activeSceneName);
    if (startingSceneManager->getStatus() != ModuleStatus::INACTIVE)
    {
        LOG(WARNING) << "Scene '" << m_activeSceneName << "' is already active";
        return;
    }

    // Launch simulation right away if the simulator starts in running mode
    if (!startSimulationPaused)
    {
        this->launchSimulation();
    }

    // start the viewer
    this->startViewer(viewerInDebugMode);
}

void
SimulationManager::startViewer(const bool debug /*= true*/)
{
    m_viewer->setRenderingMode(debug ? VTKRenderer::Mode::DEBUG : VTKRenderer::Mode::SIMULATION);

    // Start Rendering
    if (!m_viewer->isRendering())
    {
        LOG(INFO) << "Starting viewer";

        m_viewer->startRenderingLoop(); // Infinite loop

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
    if (m_status != SimulationStatus::PAUSED)
    {
        LOG(WARNING) << "SimulationManager::runSimulation() - Simulation is not paused! cannot run (un-pause) simulation";
        return;
    }
    else
    {
        LOG(INFO) << "Running simulation";
    }

    // Run scene
    m_sceneManagerMap.at(m_activeSceneName)->run();

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
    if (m_status != SimulationStatus::RUNNING)
    {
        LOG(WARNING) << "SimulationManager::pauseSimulation(): - Simulation not running, can not pause";
        return;
    }
    else
    {
        LOG(INFO) << "Pausing simulation";
    }

    m_status = SimulationStatus::PAUSING;

    // Pause scene manager module
    m_sceneManagerMap.at(m_activeSceneName)->pause();

    // Pause other modules
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
    if ((m_status != SimulationStatus::RUNNING) &&
        (m_status != SimulationStatus::PAUSED))
    {
        LOG(WARNING) << "SimulationManager::endSimulation() - Simulation already terminated!";
        return;
    }
    else
    {
        LOG(INFO) << "Ending simulation";
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
SimulationManager::startModuleInNewThread(std::shared_ptr<Module> module)
{
    m_threadMap[module->getName()] = std::thread([module] { module->start(); });
}
} // imstk
