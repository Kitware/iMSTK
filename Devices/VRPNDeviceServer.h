// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef DEVICES_VRPNDEVICESERVER_H
#define DEVICES_VRPNDEVICESERVER_H

#include<vector>

#include "Devices/DeviceInterface.h"
#include "VRPNDeviceClient.h"
#include <vrpn_Analog.h>
#include <vrpn_Tracker.h>
#include <vrpn_Button.h>

class vrpn_Connection;

class VRPNDeviceServer : public DeviceInterface
{
    VRPNDeviceServer(const VRPNDeviceServer& other) = delete;
    VRPNDeviceServer& operator=(const VRPNDeviceServer& other) = delete;

public:
    VRPNDeviceServer();
    ~VRPNDeviceServer() = default;

    bool addDeviceClient(const std::shared_ptr<VRPNDeviceClient> deviceClient,
                         const bool addFiltering=false);
    void exec() override;

private:
    vrpn_Connection *connection;

    /* Devices list
    * The best would be to store only one map of all devices
    * by using `vrpn_BaseClass` but this upcast is too many
    * times ambiguous. Using `vprn_BaseClassUnique` solves the
    * ambiguity problem but doesn't allow to run `mainloop()`.
    */
    std::map<std::string, std::shared_ptr<vrpn_Analog>> analogDevicesList;
    std::map<std::string, std::shared_ptr<vrpn_Tracker>> trackerDevicesList;
    std::map<std::string, std::shared_ptr<vrpn_Button>> buttonDevicesList;

};

#endif // VRPNSERVERDEVICE_H
