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

#include <unordered_map>
#include <vector>
#include <thread>
#include <memory>

#include "imstkScene.h"
#include "imstkModule.h"
#include "imstkSceneManager.h"
#include "imstkLogUtility.h"
#include "imstkViewer.h"

#ifdef iMSTK_USE_Vulkan
#include "imstkVulkanViewer.h"
#else
#include "imstkVTKViewer.h"
#endif

namespace imstk
{
using SimulationStatus = ModuleStatus;

template<class T>
using SceneNameMap = std::unordered_map<std::string, std::shared_ptr<T>>;


///
/// \brief Simulation manager mode
/// \note The mode is set at the time of initialization
///
enum class SimulationMode
{
    /// Simulation manager launches the simulation with a
    /// render window
    rendering = 0,

    /// Simulation manager launches the simulation without a
    /// render window but keeps looping the simulation
    runInBackgroundSync,

    /// Simulation manager launches the simulation without a
    /// render window no waiting
    runInBackgroundAsync,

    /// Simulation manager launches the simulation without a
    /// render window and returns the control
    backend
};

///
/// \brief Simulation manager configuration
///
struct simManagerConfig
{
    // logger
    std::string logPath = "./";
    std::string logFilePrefix = "simulation";

    // Name
    std::string simulationName = "imstk";

    // states
    SimulationMode simulationMode = SimulationMode::rendering;
    SimulationStatus startingStatus = SimulationStatus::RUNNING;
    bool VR_Enabled = false;
    bool startInPausedState = false;

    // 0 indicates that an optimal size will be used
    unsigned int threadPoolSize = 0;
};

///
/// \class SimulationManager
///
/// \brief This class is the manager of the simulation at the top level.
/// The simulation can contain multiple scenes. It can be launched in three
///  modes: rendering (default), runInBackground and backend
///
class SimulationManager
{
using keyPressCallback = std::function<void(void)>;

public:
    ///
    /// \brief Constructor
    ///    
    SimulationManager(const std::shared_ptr<simManagerConfig> config = std::make_shared<simManagerConfig>());

    ///
    /// \brief Default destructor
    ///
    ~SimulationManager() { this->end(); };

    ///
    /// \brief Returns the simulation status
    ///
    const SimulationStatus& getStatus() const { return m_status; };

    ///
    /// \brief Returns log worker
    ///
    std::shared_ptr<g3::LogWorker> getLogWorker() { return m_logUtil->getLogWorker(); }

    ///
    /// \brief Set number of worker threads in thread pool for parallel computation
    /// \param nThreads Size of the thread pool to set,
    /// if nThreads <= 0 then the optimal value (which typically equals to number of logical cores) will be used
    ///
    void setThreadPoolSize(const int nThreads);

    ///
    /// \brief Set number of worker threads in thread pool for parallel computation to be the optimal value,
    /// which typically equals to the number of logical cores of the machine
    ///
    void setOptimalThreadPoolSize();

    ///
    /// \brief Returns true if the scene is registered, else false
    ///
    bool isSceneRegistered(const std::string& sceneName) const;

    ///
    /// \brief Returns the scene manager given the scene
    ///
    std::shared_ptr<SceneManager> getSceneManager(const std::string& sceneName) const;
    std::shared_ptr<SceneManager> getSceneManager(std::shared_ptr<Scene> scene) const;

    ///
    /// \brief Returns the scene with a given name
    ///
    std::shared_ptr<Scene> getScene(const std::string& sceneName) const;

    ///
    /// \brief Returns the scene that is currently active
    ///
    std::shared_ptr<Scene> getActiveScene() const;

    ///
    /// \brief Create a new scene with a given name
    ///
    std::shared_ptr<Scene> createNewScene(const std::string&                  newSceneName,
                                          std::shared_ptr<SceneConfig> config = std::make_shared<SceneConfig>());

    ///
    /// \brief Create a new scene
    /// \note Scene name is automatically assigned
    ///
    std::shared_ptr<Scene> createNewScene();

    ///
    /// \brief Add a new scene with given name to the scene list
    ///
    void addScene(std::shared_ptr<Scene> newScene);

    ///
    /// \brief Remove the scene with given name from the scene list
    ///
    void removeScene(const std::string& sceneName);

    // Modules

    ///
    /// \brief Returns true if the modules is registered, else false
    ///
    bool isModuleRegistered(const std::string& moduleName) const;

    ///
    /// \brief Returns the module given the name
    ///
    std::shared_ptr<Module> getModule(const std::string& moduleName) const;

    ///
    /// \brief Add a new module with a given name
    ///
    void addModule(std::shared_ptr<Module> newModule);

    ///
    /// \brief Remove the module with a given name
    ///
    void removeModule(const std::string& moduleName);

    // Viewer
    std::shared_ptr<Viewer> getViewer() const;

    ///
    /// \brief Set the current scene to the one with the supplied name
    ///
    void setActiveScene(const std::string& newSceneName,
                        const bool         unloadCurrentScene = false);
    void setActiveScene(std::shared_ptr<Scene> scene,
                        const bool             unloadCurrentScene = false);

    ///
    /// \brief Start the simulation by initializing the active scene
    /// In Mode::backend mode, the simulation manager is initialized and
    /// the modules except the scene manager are launched in new threads
    /// and returned. In rendering and runInBackground modes the simulation manager
    /// module gets launched and an never returns
    ///
    void start(const SimulationStatus simStatus = SimulationStatus::RUNNING,
               const Renderer::Mode   renderMode = Renderer::Mode::SIMULATION);

    ///
    /// \brief Initialize the modules and the active scene
    ///
    void initialize();

    ///
    /// \brief Run the simulation from a paused state
    /// In Mode::backend mode, the simulation manager is initialized if not and returned
    ///
    void run();

    ///
    /// \brief Pause the simulation
    ///
    void pause();

    ///
    /// \brief Reset the simulation to initial state
    ///
    void reset();

    ///
    /// \brief End the simulation
    ///
    void end();

    ///
    /// \brief Advance to next frame
    ///
    void advanceFrame();

    ///
    /// \brief Return the mode of the simulation manager
    ///
    SimulationMode getMode() const { return m_config->simulationMode; }

    ///
    /// \brief Add key press callback to be used in background mode only
    /// \todo add remove function as well
    ///
    void addKeyPressCallback(keyPressCallback func, const int c);

private:
    ///
    /// \brief Create a viewer
    ///
    void createViewer(const bool enableVR);

    ///
    /// \brief Start the viewer
    ///
    void startViewer(const Renderer::Mode renderMode = Renderer::Mode::DEBUG);

    ///
    /// \brief Print user keyboard controls
    ///
    void printUserControlsInfo(const bool isRendering = true) const;

    ///
    /// \brief Start a module (refer \link  imstkModule \endlink) in new thread
    ///
    void startModuleInNewThread(std::shared_ptr<Module> module);

    ///
    /// \brief Start modules
    ///
    void startNonSceneModules();

    ///
    /// \brief Launch scene manager modules
    ///
    void launchSceneModule(std::string sceneName);

    ///
    /// \brief Pause modules
    ///
    void pauseModules();

    ///
    /// \brief End modules
    ///
    void endModules();

    ///
    /// \brief Keeps things in an infinite loop if rendering is disabled
    /// The same keys can be used to trigger PAUSE, RUNNING, RESET of the simulation
    ///
    void infiniteLoopNoRenderingMode();

    std::string m_activeSceneName = "";

    // Maps
    SceneNameMap<SceneManager> m_sceneManagerMap;
    SceneNameMap<Scene>        m_sceneMap; // used in backend mode where m_sceneManagerMap is not used
    SceneNameMap<Module>       m_modulesMap;

    std::unordered_map<std::string, std::thread> m_threadMap;

    std::shared_ptr<Viewer>     m_viewer  = nullptr;
    std::shared_ptr<LogUtility> m_logUtil = std::make_shared<LogUtility>();

    struct callbackKeyPair { int key; keyPressCallback func; };
    std::vector<callbackKeyPair> m_kepPressCallbacks;

    // states
    SimulationStatus m_status = SimulationStatus::INACTIVE;
    bool m_simulationStarted  = false;
    bool m_initialized        = false;

    std::shared_ptr<simManagerConfig> m_config;

    std::mutex m_mutex;
};
} // imstk
