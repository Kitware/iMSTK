// This file is part of the SimMedTK project.
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
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#ifndef VRPNSERVERDEVICE_H
#define VRPNSERVERDEVICE_H

#include<vector>

#include "Devices/DeviceInterface.h"

class vrpn_Analog;
class vrpn_Tracker;
class vrpn_ForceDeviceServer;
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
    std::vector<std::shared_ptr<vrpn_Analog>> analogServers;
    std::vector<std::shared_ptr<vrpn_Tracker>> trackerServers;
    std::vector<std::shared_ptr<vrpn_ForceDeviceServer>> forceServers;
    std::shared_ptr<vrpn_Connection> connection;
};

#endif // VRPNSERVERDEVICE_H
