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
#include "imstkLogUtility.h"

namespace imstk {
using SimulationStatus = ModuleStatus;

class SimulationManager
{
public:

    SimulationManager()
    {
        // Init g3logger
        m_logUtil->createLogger("simulation", "./");
    }

    ~SimulationManager() = default;

    const SimulationStatus& getStatus() const;

    // Scene
    bool                    isSceneRegistered(std::string sceneName) const;
    std::shared_ptr<Scene>  getScene(std::string sceneName) const;
    std::shared_ptr<Scene>  createNewScene(std::string newSceneName);
    std::shared_ptr<Scene>  createNewScene();
    void                    addScene(std::shared_ptr<Scene>newScene);
    void                    removeScene(std::string sceneName);

    // Simulation
    void                    startSimulation(std::string sceneName);
    void                    switchScene(std::string newSceneName,
                                        bool        unloadCurrentScene);
    void                    runSimulation();
    void                    pauseSimulation();
    void                    endSimulation();

private:

    void startModuleInNewThread(std::shared_ptr<Module>module);

    SimulationStatus m_status = SimulationStatus::INACTIVE;

    std::string m_currentSceneName;
    std::unordered_map<std::string, std::shared_ptr<Scene> > m_sceneMap;
    std::unordered_map<std::string, std::thread> m_threadMap;

    std::shared_ptr<LogUtility> m_logUtil = std::make_shared<LogUtility>();
};
}

#endif // ifndef imstkSimulationManager_h
