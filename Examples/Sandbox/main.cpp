#include <cstring>
#include <iostream>
#include <memory>
#include <thread>

#include "imstkScene.h"


int main()
{
    std::cout << "****************" << std::endl
              << "Starting Sandbox" << std::endl
              << "****************" << std::endl;

    std::shared_ptr<imstk::Scene> scene1 =
            std::make_shared<imstk::Scene>("Scene1");

    std::thread t1([scene1]{scene1->exec();});

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    scene1->pause();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    scene1->run();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    scene1->pause();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    scene1->terminate();

    t1.join();


    return 0;
}
