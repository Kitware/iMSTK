/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkOpenHapticDeviceManager.h"
#include "imstkOpenHapticDeviceClient.h"
#include "imstkLogger.h"

#include <HD/hd.h>

namespace imstk
{
std::shared_ptr<DeviceClient>
OpenHapticDeviceManager::makeDeviceClient(std::string name)
{
    auto deviceClient = std::shared_ptr<OpenHapticDeviceClient>(new OpenHapticDeviceClient(name));
    m_deviceClients.push_back(deviceClient);
    return deviceClient;
}

bool
OpenHapticDeviceManager::initModule()
{
    // \todo: Figure out how to stop module in order to reinitialize
    if (!getInit())
    {
        for (const auto& client : m_deviceClients)
        {
            client->initialize();
        }
        hdStartScheduler();
    }
    else
    {
        LOG(WARNING) << "OpenHapticDeviceManager already initialized. Reinitialization not implemented.";
    }
    return true;
}

void
OpenHapticDeviceManager::updateModule()
{
    for (int i = 0; i < m_deviceClients.size(); i++)
    {
        m_deviceClients[i]->update();
    }
}

void
OpenHapticDeviceManager::uninitModule()
{
    // \todo: Other threads could be mid update call here
    hdStopScheduler();
    for (const auto& client : m_deviceClients)
    {
        client->disable();
    }
}
} // namespace imstk