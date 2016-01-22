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

class vrpn_Connection;

class VRPNDeviceServer : public DeviceInterface
{
    VRPNDeviceServer(const VRPNDeviceServer& other) = delete;
    VRPNDeviceServer& operator=(const VRPNDeviceServer& other) = delete;

public:
    VRPNDeviceServer();
    ~VRPNDeviceServer() = default;

    void exec() override;

private:
    vrpn_Connection *connection;
};

#endif // VRPNSERVERDEVICE_H
