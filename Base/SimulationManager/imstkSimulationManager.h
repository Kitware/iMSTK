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

#include "imstkVTKRenderer.h"
#include "imstkScene.h"
#include "imstkModule.h"
#include "imstkSceneManager.h"
#include "imstkVTKViewer.h"
#include "imstkLogger.h"

namespace imstk
{

using SimulationStatus = ModuleStatus;

///
/// \class SimulationManager
///
/// \brief
///
class SimulationManager
{
public:
    ///
    /// \brief Constructor
    ///
    SimulationManager()
    {
        // Init g3logger
        m_logUtil->createLogger("simulation", "./");
    }

    ///
    /// \brief Default destructor
    ///
    ~SimulationManager() = default;

    ///
    /// \brief
    ///
    const SimulationStatus& getStatus() const;

    // Scene

    ///
    /// \brief
    ///
    bool isSceneRegistered(std::string sceneName) const;

    ///
    /// \brief
    ///
    std::shared_ptr<Scene> getScene(std::string sceneName) const;

    ///
    /// \brief
    ///
    std::shared_ptr<Scene> getCurrentScene() const;

    ///
    /// \brief
    ///
    std::shared_ptr<Scene> createNewScene(std::string newSceneName);

    ///
    /// \brief
    ///
    std::shared_ptr<Scene> createNewScene();

    ///
    /// \brief
    ///
    void addScene(std::shared_ptr<Scene> newScene);

    ///
    /// \brief
    ///
    void removeScene(std::string sceneName);

    // Modules

    ///
    /// \brief
    ///
    bool isModuleRegistered(std::string moduleName) const;

    ///
    /// \brief
    ///
    std::shared_ptr<Module> getModule(std::string moduleName) const;

    ///
    /// \brief
    ///
    void addModule(std::shared_ptr<Module> newModule);

    ///
    /// \brief
    ///
    void removeModule(std::string moduleName);

    // Viewer
    std::shared_ptr<VTKViewer> getViewer() const;

    // Simulation
    ///
    /// \brief
    ///
    void setCurrentScene(std::string newSceneName, bool unloadCurrentScene = false);
    void setCurrentScene(std::shared_ptr<Scene> scene, bool unloadCurrentScene = false);

    ///
    /// \brief
    ///
    void startSimulation(bool debug = false);

    ///
    /// \brief
    ///
    void runSimulation();

    ///
    /// \brief
    ///
    void pauseSimulation();

    ///
    /// \brief
    ///
    void endSimulation();

private:

    void startModuleInNewThread(std::shared_ptr<Module> module);

    SimulationStatus m_status = SimulationStatus::INACTIVE;

    std::string m_currentSceneName = "";
    std::unordered_map<std::string, std::shared_ptr<SceneManager>> m_sceneManagerMap;

    std::unordered_map<std::string, std::shared_ptr<Module>> m_modulesMap;

    std::unordered_map<std::string, std::thread> m_threadMap;

    std::shared_ptr<VTKViewer> m_viewer = std::make_shared<VTKViewer>(this);
    std::shared_ptr<LogUtility> m_logUtil = std::make_shared<LogUtility>();
};

} // imstk

#endif // ifndef imstkSimulationManager_h
