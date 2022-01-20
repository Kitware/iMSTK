/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#pragma once

#include "imstkDeviceClient.h"

#include "vrpn_Tracker.h"
#include "vrpn_Analog.h"
#include "vrpn_Button.h"
#include "vrpn_ForceDevice.h"

#include "imstkVRPNDeviceManager.h"

namespace imstk
{
///
/// \class VRPNDeviceClient
/// \brief This class is the receiver of the updates sent by the vrpn_server
///
/// After adding a `VRPNDeviceClient` to the `VRPNDeviceManger` the static functions
/// in this class will be called whenever new data comes in from `vrpn_server`.
///
class VRPNDeviceClient : public DeviceClient
{
public:

    ///
    /// \brief Constructor
    ///
    VRPNDeviceClient(const std::string& deviceName, VRPNDeviceType type, const std::string& ip = "localhost");

    ///
    /// \brief Destructor
    ///
    virtual ~VRPNDeviceClient() override = default;

    ///
    /// \brief VRPN call back for position and orientation data
    /// \param userData Pointer to a VRPNDeviceClient to allow updating internal data the server class
    ///                 used the device client pointer when registering
    /// \param t VRPN callback structure containing new position and orientation data
    ///
    static void VRPN_CALLBACK trackerPositionChangeHandler(void* userData, const _vrpn_TRACKERCB t);

    ///
    /// \brief VRPN call back for velocity data
    /// \param userData Pointer to a VRPNDeviceClient to allow updating internal data the server class
    ///                 used the device client pointer when registering
    /// \param v VRPN callback structure tracker velocity
    ///
    static void VRPN_CALLBACK trackerVelocityChangeHandler(void* userData, const _vrpn_TRACKERVELCB v);

    ///
    /// \brief VRPN call back analog data
    /// \param userData Pointer to a VRPNDeviceClient to allow updating internal data the server class
    ///                 used the device client pointer when registering
    /// \param a VRPN callback structure for the analog data
    ///
    static void VRPN_CALLBACK analogChangeHandler(void* userData, const _vrpn_ANALOGCB a);

    ///
    /// \brief VRPN call back for button changed (pressed or released)
    /// \param userData Pointer to a VRPNDeviceClient to allow updating internal data the server class
    ///                 used the device client pointer when registering
    /// \param b VRPN callback structure containing new button data
    ///
    static void VRPN_CALLBACK buttonChangeHandler(void* userData, const _vrpn_BUTTONCB b);

    /// \return all the types that this client is tracking
    VRPNDeviceType getType() const;

private:
    VRPNDeviceType m_type;
};
} // namespace imstk
