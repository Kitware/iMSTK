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
#include "imstkVRPNDeviceServer.h"
#include "imstkModule.h"

#include <vrpn_Configure.h>
#include <vrpn_Tracker.h>
#include <vrpn_Analog.h>
#include <vrpn_Button.h>
#include <vrpn_ForceDevice.h>
#include <unordered_map>

//
#include <vector>
#include "quat.h"

namespace imstk
{
///
/// \class VRPNDeviceClient
/// \brief Subclass of DeviceClient using VRPN
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
    /// \param userData Pointer to this to allow updating
    /// internal data
    /// \param b VRPN callback structure containing new position and
    /// orientation data
    ///
    static void VRPN_CALLBACK trackerChangeHandler(void* userData, const _vrpn_TRACKERCB t);

    ///
    /// \brief VRPN call back for position and orientation data
    /// \param userData Pointer to this to allow updating
    /// internal data
    /// \param b VRPN callback structure containing new position and
    /// orientation data
    ///
    static void VRPN_CALLBACK analogChangeHandler(void* userData, const _vrpn_ANALOGCB a);

    ///
    /// \brief VRPN call back for velocity data
    /// \param userData Pointer to this to allow updating
    /// internal data
    /// \param v VRPN callback structure containing new position and
    /// orientation data
    ///
    static void VRPN_CALLBACK velocityChangeHandler(void* userData, const _vrpn_TRACKERVELCB v);

    ///
    /// \brief VRPN call back for button changed (pressed or released)
    /// \param userData Pointer to this to allow updating
    /// internal data
    /// \param b VRPN callback structure containing new button data
    ///
    static void VRPN_CALLBACK buttonChangeHandler(void* userData, const _vrpn_BUTTONCB b);

    VRPNDeviceType getType() const;

private:
    VRPNDeviceType m_type;
};
}
