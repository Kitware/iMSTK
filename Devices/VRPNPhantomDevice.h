// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
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

#ifndef SM_VRPNPHANTOMDEVICE_H
#define SM_VRPNPHANTOMDEVICE_H

#include "Devices/VRPNDeviceClient.h"

///
/// \brief An interface for the Phantom Omni or Geomagic Touch using VRPN.
///     This is the main client code.
///
class VRPNPhantomDevice : public VRPNDeviceClient
{
public:
    ///
    /// \brief Constructor/Destructor
    ///
    VRPNPhantomDevice();
    ~VRPNPhantomDevice();

    ///
    /// \brief Open the connection to the VRPN server
    /// \return DeviceInterface::Message::Success on success and
    /// DeviceInterface::Message::Failure on failure
    ///
    DeviceInterface::Message openDevice() override;

    ///
    /// \brief Close the connection to the VRPN server
    /// \return DeviceInterface::Message::Success on success and
    /// DeviceInterface::Message::Failure on failure
    ///
    DeviceInterface::Message closeDevice() override;

    ///
    /// \brief Initialize module
    ///
    void init() override;

protected:
    ///
    /// \brief Executes the mainloop of each of the VRPN interfaces to obtain data
    /// \detail If there is new data, the callback functions will be called
    ///
    void processChanges() override;

private:
    std::shared_ptr<vrpn_ForceDevice_Remote> vrpnForce; //!< VRPN force interface

    ///
    /// \brief VRPN call back for force data
    /// \param userData Pointer to this to allow updating
    /// internal data
    /// \param f VRPN callback structure containing new force data
    ///
    static void VRPN_CALLBACK forceChangeHandler(void *userData, const vrpn_FORCECB f);

};

#endif
