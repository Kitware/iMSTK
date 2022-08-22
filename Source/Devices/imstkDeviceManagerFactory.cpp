/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkDeviceManagerFactory.h"
#include "imstkLogger.h"

#ifdef iMSTK_USE_HAPLY
#include "imstkHaplyDeviceManager.h"
#endif

#ifdef iMSTK_USE_OPENHAPTICS
#include "imstkOpenHapticDeviceManager.h"
#endif

#ifdef iMSTK_USE_VRPN
#include "imstkVRPNDeviceManager.h"
#endif

namespace imstk
{
#ifdef iMSTK_USE_HAPLY
IMSTK_REGISTER_DEVICE_MANAGER(HaplyDeviceManager);
#endif

#ifdef iMSTK_USE_OPENHAPTICS
IMSTK_REGISTER_DEVICE_MANAGER(OpenHapticDeviceManager);
#endif

#ifdef iMSTK_USE_VRPN
IMSTK_REGISTER_DEVICE_MANAGER(VRPNDeviceManager);
#endif

std::shared_ptr<DeviceManager>
DeviceManagerFactory::makeDeviceManager(const std::string typeName)
{
    if (!contains(typeName))
    {
        LOG(FATAL) << "No DeviceManager type named: " << typeName;
        return nullptr;
    }
    else
    {
        return create(typeName);
    }
}

std::shared_ptr<DeviceManager>
DeviceManagerFactory::makeDeviceManager()
{
#ifdef iMSTK_USE_HAPLY
    return makeDeviceManager("HaplyDeviceManager");
#endif
#ifdef iMSTK_USE_OPENHAPTICS
    return makeDeviceManager("OpenHapticDeviceManager");
#endif
#ifdef iMSTK_USE_VRPN
    return makeDeviceManager("VRPNDeviceManager");
#endif
    LOG(FATAL) << "Tried to make default DeviceManager but no haptic API was found";
    return nullptr;
}
} // namespace imstk