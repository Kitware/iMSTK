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

#ifndef imstkVRPNDeviceClient_h
#define imstkVRPNDeviceClient_h

#include <map>

#include "imstkDeviceClient.h"
#include "imstkModule.h"

#include <memory>

#include <vrpn_Configure.h>
#include <vrpn_Tracker.h>
#include <vrpn_Analog.h>
#include <vrpn_Button.h>
#include <vrpn_ForceDevice.h>

namespace imstk
{

///
/// \class VRPNDeviceClient
/// \brief Subclass of DeviceClient using VRPN
///
class VRPNDeviceClient : public DeviceClient, public Module
{
public:

    ///
    /// \brief Constructor
    ///
    VRPNDeviceClient(std::string deviceName, std::string ip):
        DeviceClient(deviceName, ip),
        Module(deviceName+"@"+ip)
    {}

    ///
    /// \brief Destructor
    ///
    virtual ~VRPNDeviceClient()
    {}

protected:
    ///
    /// \brief Initialize device client module
    ///
    void initModule() override;

    ///
    /// \brief Run the device client
    ///
    void runModule() override;

    ///
    /// \brief Clean the device client module
    ///
    void cleanUpModule() override;

private:

    ///
    /// \brief VRPN call back for position and orientation data
    /// \param userData Pointer to this to allow updating
    /// internal data
    /// \param b VRPN callback structure containing new position and
    /// orientation data
    ///
    static void VRPN_CALLBACK trackerChangeHandler(void *userData, const _vrpn_TRACKERCB t);

    ///
    /// \brief VRPN call back for position and orientation data
    /// \param userData Pointer to this to allow updating
    /// internal data
    /// \param b VRPN callback structure containing new position and
    /// orientation data
    ///
    static void VRPN_CALLBACK analogChangeHandler(void *userData, const _vrpn_ANALOGCB a);

    ///
    /// \brief VRPN call back for velocity data
    /// \param userData Pointer to this to allow updating
    /// internal data
    /// \param v VRPN callback structure containing new position and
    /// orientation data
    ///
    static void VRPN_CALLBACK velocityChangeHandler(void *userData, const _vrpn_TRACKERVELCB v);

    ///
    /// \brief VRPN call back for button changed (pressed or released)
    /// \param userData Pointer to this to allow updating
    /// internal data
    /// \param b VRPN callback structure containing new button data
    ///
    static void VRPN_CALLBACK buttonChangeHandler(void *userData, const _vrpn_BUTTONCB b);

    ///
    /// \brief VRPN call back for force data
    /// \param userData Pointer to this to allow updating
    /// internal data
    /// \param f VRPN callback structure containing new force data
    ///
    static void VRPN_CALLBACK forceChangeHandler(void *userData, const _vrpn_FORCECB f);

    std::shared_ptr<vrpn_Tracker_Remote> m_vrpnTracker;         //!< VRPN position/orientation interface
    std::shared_ptr<vrpn_Analog_Remote> m_vrpnAnalog;           //!< VRPN position/orientation interface
    std::shared_ptr<vrpn_Button_Remote> m_vrpnButton;           //!< VRPN button interface
    std::shared_ptr<vrpn_ForceDevice_Remote> m_vrpnForceDevice; //!< VRPN force interface

    bool m_offsetSet = false;
    Quatd m_rotOffset = Quatd::Identity();
};
}

#endif // ifndef imstkVRPNDeviceClient_h
