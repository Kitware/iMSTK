/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkHapticDeviceManager.h"
#include "imstkHapticDeviceClient.h"
#include "imstkLogger.h"

#include <HD/hd.h>

namespace imstk
{
std::shared_ptr<HapticDeviceClient>
HapticDeviceManager::makeDeviceClient(std::string name)
{
    auto deviceClient = std::shared_ptr<HapticDeviceClient>(new HapticDeviceClient(name));
    m_deviceClients.push_back(deviceClient);
    return deviceClient;
}

bool
HapticDeviceManager::initModule()
{
    for (const auto& client : m_deviceClients)
    {
        client->initialize();
    }
    hdStartScheduler();
    return true;
}

void
HapticDeviceManager::updateModule()
{
    for (int i = 0; i < m_deviceClients.size(); i++)
    {
        m_deviceClients[i]->update();
    }
}

void
HapticDeviceManager::uninitModule()
{
    // \todo: Other threads could be mid update call here
    hdStopScheduler();
    for (const auto& client : m_deviceClients)
    {
        client->disable();
    }
}
} // namespace imstk