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
HaplyDeviceManager::getInverse3PortNames()
{
    // Get all the inverse3 devices (max 10)
    std::string portNames[10];
    const int portCount = Devices::DeviceDetection::AutoDetectInverse3(portNames);
    // \note: The above detection function prints stuff. Undesirable
    std::cout << std::endl;

    std::vector<std::string> results;
    for (int i = 0; i < portCount; i++)
    {
        results.push_back(portNames[i]);
    }
    return results;
}

std::vector<std::string>
HaplyDeviceManager::getHandlePortNames()
{
    // Find Handle
    std::string portNames[10];
    const int portCount = Devices::DeviceDetection::AutoDetectHandle(portNames);

    std::vector<std::string> results;
    for (int i = 0; i < portCount; i++)
    {
        results.push_back(portNames[i]);
    }
    return results;
}

std::shared_ptr<DeviceClient>
HaplyDeviceManager::makeDeviceClient(std::string portName = "", std::string handlePortName = "")
{
    // Autodetect the first device found
    if (portName == "")
    {
        std::vector<std::string> portNames = getInverse3PortNames();
        CHECK(portNames.size() > 0) << "No Haply Inverse3 devices found";
        portName = portNames[0];
    }
    // Handle is optional
    if (handlePortName == "")
    {
        std::vector<std::string> portNames = getHandlePortNames();
        if (portNames.size() > 0)
        {
            handlePortName = portNames[0];
        }
    }

    auto deviceClient = std::shared_ptr<HaplyDeviceClient>(new HaplyDeviceClient(portName, handlePortName));
    m_deviceClients.push_back(deviceClient);
    return deviceClient;
}

bool
HaplyDeviceManager::isDevicePresent()
{
    std::vector<std::string> portNames = HaplyDeviceManager::getInverse3PortNames();
    // List the devices found
    for (const auto& portName : portNames)
    {
        if (portName != "")
        {
            return true;
        }
    }
    return false;
}

bool
HaplyDeviceManager::initModule()
{
    // \todo: Figure out how to stop module in order to reinitialize
    if (!getInit())
    {
        LOG(INFO) << "Haply HardwareAPI version " << GetLibraryVersion();

        std::vector<std::string> portNames = HaplyDeviceManager::getInverse3PortNames();
        // List the devices found
        for (const auto& portName : portNames)
        {
            LOG(INFO) << "Inverse3 device available with name: " << portName;
        }

        // List the handles found
        portNames = HaplyDeviceManager::getHandlePortNames();
        for (const auto& portName : portNames)
        {
            LOG(INFO) << "Haply Handle device available with name: " << portName;
        }
        if (portNames.size() == 0)
        {
            LOG(INFO) << "No Haply Handle device available.";
        }

        for (const auto& client : m_deviceClients)
        {
            client->initialize();
        }
        //hdStartScheduler();
    }
    else
    {
        LOG(WARNING) << "HaplyDeviceManager already initialized. Reinitialization not implemented.";
    }
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