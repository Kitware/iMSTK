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

#include <map>
#include <vector>
#include <thread>

#include <imstkScene.h>

namespace imstk {
using SimulationStatus = ModuleStatus;

class SimulationManager
{
public:

    SimulationManager()  = default;
    ~SimulationManager() = default;

    const SimulationStatus& getStatus() const;

    // Scene
    std::shared_ptr<Scene>  createNewScene(std::string newSceneName);
    std::shared_ptr<Scene>  createNewScene();
    void                    addScene(std::shared_ptr<Scene>newScene);
    void                    removeScene(std::string sceneName);
    std::shared_ptr<Scene>  getScene(std::string sceneName);
    bool                    isSceneRegistered(std::string sceneName);

    // Simulation
    void                    startSimulation(std::string sceneName);
    void                    switchScene(std::string newSceneName,
                                        bool        unloadCurrentScene);
    void                    runSimulation();
    void                    pauseSimulation();
    void                    endSimulation();

private:

    void startModuleInNewThread(std::shared_ptr<Module>module);

    std::string m_currentSceneName;
    std::map<std::string, std::shared_ptr<Scene> > m_sceneMap;
    std::map<std::string, std::thread> m_threadMap;
    SimulationStatus m_status = SimulationStatus::INACTIVE;
};
}

#endif // ifndef imstkSimulationManager_h
