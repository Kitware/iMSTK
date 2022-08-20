/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkHaplyDeviceManager.h"
#include "imstkHaplyDeviceClient.h"
#include "imstkLogger.h"

#include <HardwareAPI.h>
#include <DeviceDetection.h>

using namespace Haply::HardwareAPI;

namespace imstk
{
std::vector<std::string>
HaplyDeviceManager::getPortNames() const
{
    // Get all the inverse3 devices (max 10)
    std::string portNames[10];
    Devices::DeviceDetection::AutoDetectInverse3(portNames);
    // \note: The above detection function prints stuff. Undesirable
    std::cout << std::endl;

    std::vector<std::string> results;
    for (int i = 0; i < 10; i++)
    {
        if (portNames[i] != "")
        {
            results.push_back(portNames[i]);
        }
    }
    return results;
}

std::shared_ptr<DeviceClient>
HaplyDeviceManager::makeDeviceClient(std::string portName)
{
    // Autodetect the first device found
    if (portName == "")
    {
        std::vector<std::string> portNames = getPortNames();
        CHECK(portNames.size() > 0) << "No Haply Inverse3 devices found";
        portName = portNames[0];
    }

    auto deviceClient = std::shared_ptr<HaplyDeviceClient>(new HaplyDeviceClient(portName));
    m_deviceClients.push_back(deviceClient);
    return deviceClient;
}

bool
HaplyDeviceManager::initModule()
{
    LOG(INFO) << "Haply HardwareAPI version " << GetLibraryVersion();

    std::vector<std::string> portNames = getPortNames();
    // List the devices found
    for (const auto& portName : portNames)
    {
        if (portName != "")
        {
            LOG(INFO) << "Inverse3 device available with name: " << portName;
        }
    }

    for (const auto& client : m_deviceClients)
    {
        client->initialize();
    }
    //hdStartScheduler();
    return true;
}

void
HaplyDeviceManager::updateModule()
{
    for (const auto& client : m_deviceClients)
    {
        client->update();
    }
}

void
HaplyDeviceManager::uninitModule()
{
    // \todo: Other threads could be mid update call here
    //hdStopScheduler();
    for (const auto& client : m_deviceClients)
    {
        client->disable();
    }
}
} // namespace imstk