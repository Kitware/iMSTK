#ifndef imstkSimulationManager_h
#define imstkSimulationManager_h

#include <map>
#include <vector>
#include <thread>

#include <imstkScene.h>


namespace imstk {

class SimulationManager
{
public:
    ~SimulationManager() = default;

private:
    SimulationManager() = default;

    std::map<std::string, std::shared_ptr<Scene>> sceneMap;
    std::size_t activeSceneID;
    std::vector<std::thread> threads;
};

}

#endif
