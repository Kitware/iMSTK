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

#ifndef imstkSimulationManager_h
#define imstkSimulationManager_h

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

///
/// \class SimulationManager
///
/// \brief This class manages the overall simulation. The simulation can contain multiple scenes.
///
class SimulationManager
{
public:
    ///
    /// \brief Constructor
    ///
    SimulationManager(bool enableVR = false);

    ///
    /// \brief Default destructor
    ///
    ~SimulationManager() = default;

    ///
    /// \brief Returns the simulation status
    ///
    const SimulationStatus& getStatus() const;

    // Scene

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
    std::shared_ptr<Scene> createNewScene(const std::string& newSceneName);
    std::shared_ptr<Scene> createNewScene(std::string&& newSceneName);

    ///
    /// \brief Create a new scene with default name
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

    // Simulation

    ///
    /// \brief Set the current scene to the one with the supplied name
    ///
    void setActiveScene(const std::string& newSceneName,
                        const bool unloadCurrentScene = false);
    void setActiveScene(std::shared_ptr<Scene> scene,
                        const bool unloadCurrentScene = false);

    ///
    /// \brief Start the viewer
    ///
    void startViewer(const bool debug = true);

    ///
    /// \brief Print user keyboard controls
    ///
    void printUserControlsInfo();

    ///
    /// \brief Launch simulation for the first time.
    /// 1. Initialize the active scene if not initialized already.
    /// 2. Launches separate threads for each module.
    ///
    void launchSimulation();

    ///
    /// \brief Start the simulation by initializing the active scene
    ///
    void startSimulation(const bool startSimulationPaused = true,
                         const bool viewerInDebugMode = false);

    ///
    /// \brief Run the simulation from a paused state
    ///
    void runSimulation();

    ///
    /// \brief Pause the simulation
    ///
    void pauseSimulation();

    ///
    /// \brief End the simulation
    ///
    void resetSimulation();

    ///
    /// \brief
    ///
    void endSimulation();

private:

    void startModuleInNewThread(std::shared_ptr<Module> module);

    SimulationStatus m_status = SimulationStatus::INACTIVE;

    std::string m_activeSceneName = "";
    std::unordered_map<std::string, std::shared_ptr<SceneManager>> m_sceneManagerMap;

    std::unordered_map<std::string, std::shared_ptr<Module>> m_modulesMap;

    std::unordered_map<std::string, std::thread> m_threadMap;

    std::shared_ptr<Viewer> m_viewer;
    std::shared_ptr<LogUtility> m_logUtil = std::make_shared<LogUtility>();
};
} // imstk

#endif // ifndef imstkSimulationManager_h
