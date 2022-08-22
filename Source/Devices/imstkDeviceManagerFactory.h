/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkFactory.h"

namespace imstk
{
class DeviceClient;
class DeviceManager;

/// \brief Manages and generates the DeviceManager's.
///
/// The factory is a singleton and can be accessed anywhere. Given a
/// name of a DeviceManager this will generate the appropriate object
/// that should be executed. By default the class name is the name that is
/// used to look up the DeviceManager.
/// ie: "HapticDeviceManager" -> HapticDeviceManager
/// The generation Will fail if the name is not known to the factory.
///
/// There are multiple ways to register a DeviceManager,
/// preferred should be ...
/// \code
/// IMSTK_REGISTER_DEVICE_MANAGER(typeName)
/// \endcode
/// will register the delegate for the class-name of the algorithm,
/// this will satisfy the default mechanism
/// If want to register a custom delegate with more customization you can use
/// \code
/// DeviceManagerRegistrar<delegateType> registrar("LookupTypeName");
/// \endcode
///
class DeviceManagerFactory : public ObjectFactory<std::shared_ptr<DeviceManager>>
{
public:
    ///
    /// \brief Attempts to create a new DeviceManager by type name
    /// \param typeName name of the class to look up
    static std::shared_ptr<DeviceManager> makeDeviceManager(const std::string typeName);

    ///
    /// \brief Attempts to create a new DeviceManager by whichever is default
    /// If multiple haptic managers are built with, the preference is:
    /// Haply > OpenHaptics > VRPN
    /// \todo: Extend based on device availability, multi device support
    ///
    static std::shared_ptr<DeviceManager> makeDeviceManager();
};

///
/// \brief Auto registration class
/// \tparam T type of the class to register
template<typename T>
using DeviceManagerRegistrar = SharedObjectRegistrar<DeviceManager, T>;

#define IMSTK_REGISTER_DEVICE_MANAGER(objType) DeviceManagerRegistrar<objType> _imstk_registerdevicemanager ## objType(#objType)
} // namespace imstk
