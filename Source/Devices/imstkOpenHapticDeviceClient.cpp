/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkOpenHapticDeviceClient.h"
#include "imstkLogger.h"

namespace imstk
{
void
OpenHapticDeviceClient::initialize()
{
    m_trackingEnabled = true;
}

void
OpenHapticDeviceClient::update()
{
    std::vector<std::pair<int, int>> localEvents;
    m_dataLock.lock();
    std::swap(m_events, localEvents);
    m_dataLock.unlock();

    for (const auto& item : localEvents)
    {
        postEvent(ButtonEvent(OpenHapticDeviceClient::buttonStateChanged(), item.first, item.second));
    }
}

void
OpenHapticDeviceClient::disable()
{
}


} // namespace imstk