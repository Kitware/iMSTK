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
#include "imstkCollisionGraph.h"
#include "imstkCamera.h"
#include "imstkThreadManager.h"
#include "imstkSceneManager.h"
#include "imstkViewer.h"
#include "imstkTimer.h"
#include "imstkLogger.h"
#include "imstkScene.h"

#include <string>

#include "imstkLogger.h"

namespace imstk
{
SimulationManager::SimulationManager(const std::shared_ptr<simManagerConfig> config) : m_config(config)
{
    // Initialize the logger and add optional sinks
    auto logger = Logger::getInstance();
    if (config->enableFileLogging)
    {
        m_fileSinkHandle = logger->addFileSink(m_config->logFilePrefix, m_config->logPath);
    }

    if (config->enableStdoutLogging)
    {
        m_stdSinkHandle = logger->addStdoutSink();
    }

    // create viewer if the selected mode is 'rendering'
    if (m_config->simulationMode == SimulationMode::Rendering)
    {
        createViewer(m_config->VR_Enabled);
    }
}

void
SimulationManager::createViewer(const bool enableVR)
{
#ifdef iMSTK_USE_Vulkan
    m_viewer = std::make_shared<VulkanViewer>(this, enableVR);
#else

#ifdef iMSTK_ENABLE_VR
    m_viewer = std::make_shared<VTKViewer>(this, enableVR);
#else

    LOG_IF(FATAL, enableVR) << "Can not run VR simulation without iMSTK_ENABLE_VR";

    m_viewer = std::make_shared<VTKViewer>(this, false);
    m_viewer->setWindowTitle(m_config->simulationName);
#endif
#endif
}

void
SimulationManager::setThreadPoolSize(const int nThreads)
{
    m_config->threadPoolSize = nThreads;
    if (nThreads <= 0)
    {
        setOptimalThreadPoolSize();
    }
    else
    {
        ParallelUtils::ThreadManager::setThreadPoolSize(static_cast<size_t>(nThreads));
    }
}

void
SimulationManager::setOptimalThreadPoolSize()
{
    ParallelUtils::ThreadManager::setOptimalParallelism();
}

bool
SimulationManager::isSceneRegistered(const std::string& sceneName) const
{
    return m_sceneMap.find(sceneName) != m_sceneMap.end();
}

std::shared_ptr<SceneManager>
SimulationManager::getSceneManager(const std::string& sceneName) const
{
    if (m_config->simulationMode == SimulationMode::Backend)
    {
        LOG(WARNING) << "The simulation manager is in backend mode. No scene managers were created!";
        return nullptr;
    }

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
    LOG_IF(FATAL, !scene) << "SimulationManager::getSceneManager - Scene supplied is not valid!";

    return this->getSceneManager(scene->getName());
}

std::shared_ptr<Scene>
SimulationManager::getScene(const std::string& sceneName) const
{
    return m_sceneMap.find(sceneName) != m_sceneMap.end() ? m_sceneMap.at(sceneName) : nullptr;
}

std::shared_ptr<Scene>
SimulationManager::getActiveScene() const
{
    return this->getScene(m_activeSceneName);
}

std::shared_ptr<Scene>
SimulationManager::createNewScene(const std::string& newSceneName, std::shared_ptr<SceneConfig> config)
{
    // check if there is already a scene by that name
    LOG_IF(FATAL, this->isSceneRegistered(newSceneName))
        << "Can not create new scene: '" << newSceneName
        << "' is already registered in this simulation\n"
        << "You can create a new scene using an unique name";

    auto newScene = std::make_shared<Scene>(newSceneName, config);

    m_mutex.lock();

    m_sceneMap[newScene->getName()] = newScene;
    if (m_config->simulationMode != SimulationMode::Backend)
    {
        m_sceneManagerMap[newScene->getName()] = std::make_shared<SceneManager>(newScene);
    }

    m_mutex.unlock();

    LOG(INFO) << "New scene added: " << newScene->getName();

    return newScene;
}

std::shared_ptr<Scene>
SimulationManager::createNewScene()
{
    m_mutex.lock();

    int id = 0;
    id = (int)m_sceneMap.size() + 1;

    m_mutex.unlock();

    std::string newSceneName = "Scene_" + std::to_string(id);

    return this->createNewScene(newSceneName);
}

void
SimulationManager::addScene(std::shared_ptr<Scene> newScene)
{
    const std::string newSceneName = newScene->getName();

    if (this->isSceneRegistered(newSceneName))
    {
        LOG(WARNING) << "Can not add scene: '" << newSceneName
                     << "' is already registered in this simulation\n"
                     << "Set this scene name to a unique name first";
        return;
    }

    m_mutex.lock();

    // create a scene manager if not 'backend' mode
    if (m_config->simulationMode != SimulationMode::Backend)
    {
        m_sceneManagerMap[newSceneName] = std::make_shared<SceneManager>(newScene);
    }

    // add new scene to the unordered map
    m_sceneMap[newSceneName] = newScene;

    m_mutex.unlock();

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

    // If the scene to be removed is the currently active in the rendering mode, return with warning
    if (m_activeSceneName == sceneName && m_config->simulationMode == SimulationMode::Rendering)
    {
        LOG(WARNING) << "Cannot remove the active scene that is currently rendered";
        return;
    }

    m_mutex.lock();

    // remove the scene manager and the scene
    if (m_config->simulationMode != SimulationMode::Backend)
    {
        m_sceneManagerMap.erase(sceneName);
    }
    m_sceneMap.erase(sceneName);

    m_mutex.unlock();

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

    m_mutex.lock();
    m_modulesMap[newModuleName] = newModule;
    m_mutex.unlock();
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
    m_mutex.lock();
    m_modulesMap.erase(moduleName);
    m_mutex.unlock();
    LOG(INFO) << "Module removed: " << moduleName;
}

std::shared_ptr<Viewer>
SimulationManager::getViewer() const
{
    LOG_IF(WARNING, (m_config->simulationMode != SimulationMode::Rendering)) << "The simulation is not in rendering mode!";

    return m_viewer;
}

void
SimulationManager::setActiveScene(std::shared_ptr<Scene> scene,
                                  const bool             unloadCurrentScene /*= false*/)
{
    this->setActiveScene(scene->getName(), unloadCurrentScene);
}

void
SimulationManager::setActiveScene(const std::string& newSceneName,
                                  const bool         unloadCurrentScene /*= false*/)
{
    LOG(INFO) << "Setting scene '" << newSceneName << "' as active";

    // check if the scene is registered
    if (!this->isSceneRegistered(newSceneName))
    {
        LOG(WARNING) << "Scene '" << newSceneName
                     << "' not registered! Please register before setting active";
        return;
    }

    // check if the scene is already active
    if (newSceneName == m_activeSceneName)
    {
        LOG(INFO) << "Scene '" << newSceneName << "' is already active!";
        return;
    }

    auto newScene = this->getScene(newSceneName);

    // Initialize the scene if not done so already
    if (!newScene->isInitialized())
    {
        newScene->initialize();
    }

    if (m_viewer)
    {
        // Update viewer with the new scene
        m_viewer->setActiveScene(newScene);

        // If not yet rendering: update current scene and return
        if (!m_viewer->isRendering())
        {
            m_activeSceneName = newSceneName;
            return;
        }
    }

    // If rendering and simulation not active:
    // render scene in debug, update current scene, and return
    if (m_status == SimulationStatus::Inactive)
    {
        if (m_viewer)
        {
            m_viewer->setRenderingMode(Renderer::Mode::Debug);
        }
        m_activeSceneName = newSceneName;
        return;
    }

    if (m_viewer)
    {
        // If rendering and simulation active:
        // render scene in simulation mode, and update simulation
        m_viewer->setRenderingMode(Renderer::Mode::Simulation);
    }

    // Stop/Pause currently running scene
    if (m_config->simulationMode != SimulationMode::Backend)
    {
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
        if (m_status != SimulationStatus::Paused)
        {
            this->launchSceneModule(newSceneName);
            m_sceneManagerMap.at(newSceneName)->run();
        }
    }
    m_activeSceneName = newSceneName;
}

void
SimulationManager::initialize()
{
    if (m_initialized)
    {
        return;
    }

    // Do some checks
    if (m_status == SimulationStatus::Running)
    {
        LOG(WARNING) << "Simulation already running!";
        return;
    }

    // Initialize all the scenes
    for (const auto& it : m_sceneMap)
    {
        // do not initialize of not active scene
        if (!it.second->getConfig()->lazyInitialization)
        {
            if (!it.second->initialize())
            {
                LOG(WARNING) << "Unable to initialize the scene - " << it.first;
                return;
            }
            else
            {
                LOG(WARNING) << "Scene '" << it.first << "' initialized";
            }
        }
    }

    m_initialized = true;
}

void
SimulationManager::launchSceneModule(std::string sceneName)
{
    auto scene = this->getScene(sceneName);

    LOG_IF(WARNING, !scene) << "Scene '" << sceneName << "' not found!";

    const auto sceneMan = m_sceneManagerMap.at(sceneName);
    if (scene->getConfig()->trackFPS)
    {
        sceneMan->enableFrameCount();
    }

    if (sceneMan->getStatus() == ModuleStatus::Inactive)
    {
        this->startModuleInNewThread(sceneMan);
    }
}

void
SimulationManager::startNonSceneModules()
{
    // Start modules (except the scene manager module) in separate threads
    for (const auto& pair : m_modulesMap)
    {
        this->startModuleInNewThread(pair.second);
    }

    if (m_config->simulationMode == SimulationMode::Backend)
    {
        return;
    }
}

void
SimulationManager::start(const SimulationStatus simStatus /*= SimulationStatus::RUNNING*/,
                         const Renderer::Mode   renderMode /*= Renderer::Mode::SIMULATION*/)
{
    // check if there is an active scene
    LOG_IF(WARNING, !this->getActiveScene()) << "No valid active scene! Simulation canceled";

    setThreadPoolSize(m_config->threadPoolSize);
    if (!m_initialized)
    {
        this->initialize();
    }

    // start modules other than scene modules
    this->startNonSceneModules();

    if (m_config->simulationMode == SimulationMode::Backend) // returns in backend mode
    {
        return;
    }
    else
    {
        LOG_IF(FATAL, m_activeSceneName.empty())
            << "Cannot start simulation without active scene in rendering and background modes!";
    }

    if (m_status != SimulationStatus::Inactive)
    {
        LOG(INFO) << "Simulation already active";
        return;
    }

    if (m_activeSceneName != "")
    {
        auto startingSceneManager = m_sceneManagerMap.at(m_activeSceneName);
        if (startingSceneManager->getStatus() != ModuleStatus::Inactive)
        {
            return;
        }
    }

    if (simStatus != SimulationStatus::Paused)
    {
        if (m_sceneManagerMap.at(m_activeSceneName)->getStatus() == ModuleStatus::Inactive)
        {
            launchSceneModule(m_activeSceneName);
        }
        m_sceneManagerMap.at(m_activeSceneName)->run();
        m_status = SimulationStatus::Running;
    }
    else
    {
        m_status = SimulationStatus::Paused;
    }

    m_simulationStarted = true;

    // Note: This never returns until the viewer is terminated
    if (m_config->simulationMode == SimulationMode::Rendering)
    {
        // start the viewer
        this->startViewer(renderMode);
    }

    if (m_config->simulationMode == SimulationMode::RunInBackgroundSync) // never returns
    {
        this->printUserControlsInfo(false);
        this->infiniteLoopNoRenderingMode();
        this->end();
    }
}

void
SimulationManager::infiniteLoopNoRenderingMode()
{
    while (this->getStatus() == SimulationStatus::Running
           || this->getStatus() == SimulationStatus::Paused)
    {
        const auto c = getchar();
        if (c == 'e' || c == 'E')
        {
            break;
        }

        if (c == 'r' || c == 'R')
        {
            this->reset();
            continue;
        }

        if (c == ' ')
        {
            if (this->getStatus() == SimulationStatus::Running)
            {
                this->pause();
                continue;
            }

            if (this->getStatus() == SimulationStatus::Paused)
            {
                this->run();
                continue;
            }
        }

        // conditionally execute callback functions
        for (auto list : m_kepPressCallbacks)
        {
            if (c == list.key)
            {
                list.func();
            }
        }
    }
}

void
SimulationManager::startViewer(const Renderer::Mode renderMode /*= Renderer::Mode::DEBUG*/)
{
    m_viewer->setRenderingMode(renderMode);

    // Start Rendering
    if (!m_viewer->isRendering())
    {
        this->printUserControlsInfo();

        LOG(INFO) << "Starting viewer";

        m_viewer->startRenderingLoop(); // Infinite loop

        LOG(INFO) << "Closing viewer";

        // End simulation if active when loop exits
        if (m_status != SimulationStatus::Inactive)
        {
            LOG(INFO) << "Ending simulation";
            this->end();
        }
    }
}

void
SimulationManager::printUserControlsInfo(const bool isRendering) const
{
    if (isRendering)
    {
        std::cout <<
            "\n------------------------\n" <<
            "     User controls\n" <<
            "------------------------\n" <<
            "<space> - pause or un-pause simulation\n" <<
            "  R/r   - reset simulation\n" <<
            "  D/d   - toggle between debug and simulation rendering modes\n" <<
            "  P/p   - Display render frame rate on the screen\n" <<
            "------------------------\n\n" << std::endl;
    }
    else
    {
        std::cout <<
            "\n------------------------------------------------\n" <<
            "     No-rendering mode user controls\n" <<
            "------------------------------------------------\n" <<
            "<space> + <return> - pause or unpause simulation\n" <<
            "  R/r + <return>   - reset simulation\n" <<
            "  E/e + <return>   - end simulation\n" <<
            "------------------------------------------------\n\n" << std::endl;
    }
}

void
SimulationManager::run()
{
    if (m_config->simulationMode == SimulationMode::Backend)
    {
        LOG(INFO) << "Simulation cannot be run in backend mode";
        return;
    }

    if (m_status != SimulationStatus::Paused)
    {
        LOG(INFO) << "Simulation is not paused! cannot run (un-pause) simulation";
        return;
    }

    LOG(INFO) << "Running simulation";

    if (!m_simulationStarted)
    {
        if (!m_initialized)
        {
            this->initialize();
        }

        this->startNonSceneModules();
    }

    const auto sceneMan = m_sceneManagerMap.at(m_activeSceneName);
    if (sceneMan->getStatus() == ModuleStatus::Inactive)
    {
        launchSceneModule(m_activeSceneName);
    }

    // Run scene manager
    sceneMan->run();

    // Run modules
    for (const auto& mod : m_modulesMap)
    {
        mod.second->run();
    }

    // Update simulation status
    m_status = SimulationStatus::Running;
}

void
SimulationManager::pause()
{
    if (m_config->simulationMode == SimulationMode::Backend)
    {
        LOG(INFO) << "Simulation manager is in backend mode and hence pause doesn't make sense!";
        return;
    }

    if (m_status != SimulationStatus::Running)
    {
        LOG(WARNING) << "Simulation not running, can not pause";
        return;
    }

    m_status = SimulationStatus::Pausing;

    // Pause scene managers
    this->pauseModules();

    // Update simulation status
    m_status = SimulationStatus::Paused;

    LOG(INFO) << "Simulation is paused";
}

void
SimulationManager::pauseModules()
{
    // Pause scene managers
    if (m_config->simulationMode != SimulationMode::Backend)
    {
        m_sceneManagerMap.at(m_activeSceneName)->pause();
    }

    // Pause other modules
    for (const auto& pair : m_modulesMap)
    {
        (pair.second)->pause();
    }
}

void
SimulationManager::reset()
{
    LOG(INFO) << "Resetting simulation";

    // Reset all scenes
    for (const auto s : m_sceneMap)
    {
        if (s.second->isInitialized())
        {
            s.second->reset();

            if (m_status == SimulationStatus::Paused)
            {
                s.second->resetSceneObjects();
            }
        }
    }
}

void
SimulationManager::end()
{
    endModules();

    // Update simulation status
    m_status = SimulationStatus::Inactive;
}

void
SimulationManager::endModules()
{
    m_status = SimulationStatus::Terminating;

    // End modules
    for (const auto& pair : m_modulesMap)
    {
        (pair.second)->end();
        m_threadMap.at(pair.first).join();
    }

    // End all scene managers (if any)
    for (auto pair : m_sceneManagerMap)
    {
        std::string  sceneName   = pair.first;
        ModuleStatus sceneStatus = pair.second->getStatus();

        if (sceneStatus != ModuleStatus::Inactive)
        {
            m_sceneManagerMap.at(sceneName)->end();
            m_threadMap.at(sceneName).join();
        }
    }
}

void
SimulationManager::advanceFrame()
{
    if (m_initialized)
    {
        if (auto activeScene = this->getActiveScene())
        {
            activeScene->advance();
        }
    }
    else
    {
        LOG(WARNING) << "Simulation manager not initialized! call initialize before advancing frame";
    }
}

void
SimulationManager::startModuleInNewThread(std::shared_ptr<Module> module)
{
    m_threadMap[module->getName()] = std::thread([module] { module->start(); });
}

void
SimulationManager::addKeyPressCallback(keyPressCallback func, const int c)
{
    m_kepPressCallbacks.push_back(callbackKeyPair { c, func });
}
} // imstk
