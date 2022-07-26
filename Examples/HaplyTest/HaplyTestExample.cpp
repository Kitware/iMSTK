// ConsoleApplication.cpp : This file contains the 'main' function. Program
// execution begins and ends there.
//
#include <imstkParallelUtils.h>
#include <imstkParallelFor.h>

#include <string.h>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "HardwareAPI.h"
#include "imstkMath.h"

#include "imstkHaplyDeviceClient.h"
#include "imstkHaplyDeviceManager.h"

#include "imstkHapticDeviceClient.h"
#include "imstkHapticDeviceManager.h"

#include "imstkSimulationManager.h"
#include "imstkSceneManager.h"
#include "imstkScene.h"

using namespace imstk;

int
test1()
{
    Logger::startLogger();

    char* portName;

    std::string portNames[256];
    auto        nbport =
        Haply::HardwareAPI::Devices::DeviceDetection::AutoDetectInverse3(
            portNames);
    printf("Found %d ports\n", nbport);
    if (nbport > 0)
    {
        int index = nbport - 1;
        portName = _strdup(portNames[index].c_str());
    }
    else
    {
        std::cout << "No Inverse3 found" << std::endl;
        return -1;
    }

    printf("Using port %s\n", portName);
    Haply::HardwareAPI::IO::SerialStream  serial_stream(portName, false);
    char                                  open_return = serial_stream.OpenDevice();
    Haply::HardwareAPI::Devices::Inverse3 inverse3(&serial_stream);

    inverse3.SendDeviceWakeup();
    inverse3.ReceiveDeviceInfo();

    /* std::cout << std::endl << "Press ENTER to continue . . .";
     std::cin.get();*/

    Vec3f pos   = Vec3f::Zero();
    Vec3f vel   = Vec3f::Zero();
    Vec3f force = Vec3f::Zero();

    // Bounds ~ -0.5, 0.5
    while (true)
    {
        inverse3.SendEndEffectorForce(force.data());
        inverse3.ReceiveEndEffectorState(pos.data(), vel.data());
        //Vec3f fixedPos = Vec3f(pos[1], pos[2], pos[0]);
        //printf("pos: %f, %f, %f\n", fixedPos[0], fixedPos[1], fixedPos[2]);

        double planePos = 0.1;
        double ks       = 5000.0;
        if (pos[2] < planePos)
        {
            force[2] = (planePos - pos[2]) * ks;
        }
        else
        {
            force[2] = 0.0;
        }
        //printf("Applying force: %f\n", force[2]);
    }
}

void
test2()
{
    auto manager = std::make_shared<HaplyDeviceManager>();
    auto client  = manager->makeDeviceClient();

    manager->init();

    while (true)
    {
        manager->update();

        const Vec3d& pos      = client->getPosition();
        double       planePos = 0.1;
        double       ks       = 5000.0;
        if (pos[2] < planePos)
        {
            client->setForce(Vec3d(0.0, 0.0, (planePos - pos[2]) * ks));
        }
        else
        {
            client->setForce(Vec3d::Zero());
        }
    }

    manager->uninit();
}

void
test2_1()
{
    auto simulationManager = std::make_shared<SimulationManager>();

    auto scene = std::make_shared<Scene>("Scene");
    auto sceneManager = std::make_shared<SceneManager>();
    sceneManager->setActiveScene(scene);
    simulationManager->addModule(sceneManager);

    auto hapticsManager = std::make_shared<HaplyDeviceManager>();
    simulationManager->addModule(hapticsManager);
    std::shared_ptr<HaplyDeviceClient> client = hapticsManager->makeDeviceClient();

    connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
        {
        });

    simulationManager->start();
}

void
test3()
{
    auto manager = std::make_shared<HapticDeviceManager>();
    auto client  = manager->makeDeviceClient();

    manager->init();

    // Bounds the space ~ -100.0, 100.0
    while (true)
    {
        manager->update();

        const Vec3d& pos      = client->getPosition();
        double       planePos = 0.0;
        double       ks       = 5000.0 / 2500.0;
        if (pos[1] < planePos)
        {
            client->setForce(Vec3d(0.0, (planePos - pos[1]) * ks, 0.0));
        }
        else
        {
            client->setForce(Vec3d::Zero());
        }
        printf("pos: %f, %f, %f\n", pos[0], pos[1], pos[2]);
    }

    manager->uninit();
}

int
main(int argc, char* argv[])
{
    Logger::startLogger();

    test1();

    /*std::thread thread2([]() { test3(); });
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::thread thread1([]() { test2(); });

    thread1.join();
    thread2.join();*/
}