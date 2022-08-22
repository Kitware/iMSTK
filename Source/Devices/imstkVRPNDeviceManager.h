/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkDeviceManager.h"
#include "imstkDeviceClient.h"

#include <map>

//VRPN
#include <vrpn_Connection.h>
#include <vrpn_MainloopContainer.h>

#include "quat.h"

namespace imstk
{
class VRPNDeviceClient;

using VRPNDeviceType = int;
const VRPNDeviceType VRPNAnalog  = 0x1;
const VRPNDeviceType VRPNButton  = 0x2;
const VRPNDeviceType VRPNTracker = 0x4;
const VRPNDeviceType VRPNForce   = 0x8;

///
/// \class VRPNDeviceManager
///
/// \brief connects to the vrpn.exe server and lets iMSTK attach devices to the server
///
/// This module enables communication with VRPN and lets us map VRPN devices to the
/// imstk DeviceClient interface. The VRPN server is external to iMSTK and needs to
/// be run from the command line `vrpn_server`, when using the default vrpn.cfg file needs to be
/// edited represent the projects requirements.
/// For more information on VRPN see the wiki https://github.com/vrpn/vrpn/wiki
///
class VRPNDeviceManager : public DeviceManager
{
public:

    ///
    /// \brief Constructor
    /// \param machine The ip address of the machine where the vrpn server is running, "localhost" and "loopback"
    ///                are also options
    /// \param port The port that the vrpn server is listening to (can set on the command line)
    ///
    VRPNDeviceManager(const std::string& machine = "localhost", int port = vrpn_DEFAULT_LISTEN_PORT_NO);

    ~VRPNDeviceManager() override = default;

    IMSTK_TYPE_NAME(VRPNDeviceManager)

    void addDeviceClient(std::shared_ptr<VRPNDeviceClient> client);

    ///
    /// \brief Creates a client from the given parameters
    /// \param deviceName The name of the device that you want, it must match (case-sensitive) with the
    ///                   name in the selected `vrpn.cfg` file
    /// \param deviceType A binary combination of the parameters that should be updated. Note that this
    ///                   is a request, if the server doesn't supply the requested updates no changes
    ///                   will be observable
    ///
    std::shared_ptr<DeviceClient> makeDeviceClient(const std::string& deviceName, VRPNDeviceType deviceType);
    std::shared_ptr<DeviceClient> makeDeviceClient(const std::string deviceName) override
    {
        return makeDeviceClient(deviceName, VRPNAnalog);
    }

protected:
    ///
    /// \brief Initialize the server module
    ///
    bool initModule() override;

    ///
    /// \brief Run the server module
    ///
    void updateModule() override;

    ///
    /// \brief Clean the server module
    ///
    void uninitModule() override;

private:

    const std::string m_machine;                                       ///< machine name or IP
    const int m_port;                                                  ///< connection port

    struct Client
    {
        std::shared_ptr<VRPNDeviceClient> client;
        VRPNDeviceType type;
    };

    std::map<VRPNDeviceType, std::vector<std::shared_ptr<VRPNDeviceClient>>> m_deviceMap; ///< keeping track of clients
    std::unique_ptr<vrpn_MainloopContainer> m_deviceConnections;                          ///< VRPN device connections
};
} // imstk
