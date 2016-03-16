#include <cstring>
#include <iostream>
#include <memory>
#include <thread>

#include "imstkSimulationManager.h"

int main()
{
    std::cout << "****************" << std::endl
              << "Starting Sandbox" << std::endl
              << "****************" << std::endl;

    // Creating SimulationManager
    std::shared_ptr<imstk::SimulationManager> sdk =
        std::make_shared<imstk::SimulationManager>();

    std::cout << "-- Test add scenes" << std::endl;

    // Create scene and add it (scene1)
    std::shared_ptr<imstk::Scene> scene1 =
        std::make_shared<imstk::Scene>("scene1");

    scene1->setLoopDelay(500);
    sdk->addScene(scene1);

    // Create new scene through sdk (scene2)
    sdk->createNewScene("scene2");
    std::shared_ptr<imstk::Scene> scene2 = sdk->getScene("scene2");
    scene2->setLoopDelay(500);

    // Create new scene through sdk (auto : "Scene_X")
    std::shared_ptr<imstk::Scene> scene3 = sdk->createNewScene();

    // Remove scene3
    sdk->removeScene("Scene_3");

    // Test switch
    std::cout << "-- Test scene switch" << std::endl;
    sdk->startSimulation("scene1");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    sdk->switchScene("scene2", false);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    sdk->switchScene("scene1", true);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    sdk->endSimulation();

    // Test pause/run
    std::cout << "-- Test simulation pause/run" << std::endl;
    sdk->startSimulation("scene2");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    sdk->pauseSimulation();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    sdk->runSimulation();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    sdk->pauseSimulation();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    sdk->endSimulation();

    // Quit
    while (sdk->getStatus() != imstk::SimulationStatus::INACTIVE) {}
    return 0;
}
