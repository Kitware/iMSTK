// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
// Copyright (c) Kitware, Inc.
//
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

#ifndef DEVICES_VRPNDEVICECLIENT_H
#define DEVICES_VRPNDEVICECLIENT_H

#include "Devices/DeviceInterface.h"

#include <thread>
#include <algorithm>
#include <string>
#include <memory>

#include "Core/Vector.h"
#include "Core/Quaternion.h"

#include <vrpn_Configure.h>             // for VRPN_CALLBACK

class vrpn_Tracker_Remote;
class vrpn_Button_Remote;
class vrpn_Analog_Remote;

typedef struct _vrpn_BUTTONCB vrpn_BUTTONCB;
typedef struct _vrpn_TRACKERVELCB vrpn_TRACKERVELCB;
typedef struct _vrpn_TRACKERCB vrpn_TRACKERCB;
typedef struct _vrpn_ANALOGCB vrpn_ANALOGCB;

namespace imstk {

// Add support for VRPN devices by adding a DeviceType
// here and constructing the connection in VRPNDeviceServer.cpp
enum class DeviceType { SPACE_EXPLORER_3DCONNEXION,
                        NAVIGATOR_3DCONNEXION,
                        RAZER_HYDRA,
                        XKEYS_XK3,
                        PHANTOM_OMNI,
                        OSVR_HDK
                        };

class VRPNDeviceClient: public DeviceInterface
{
public:
    ///
    /// \brief Constructor/Destructor
    ///
    VRPNDeviceClient(DeviceType deviceType, std::string deviceURL);
    virtual ~VRPNDeviceClient();

    ///
    /// \brief Open the connection to the VRPN server
    /// \return DeviceInterface::Message::Success on success and
    /// DeviceInterface::Message::Failure on failure
    ///
    virtual DeviceInterface::Message openDevice() override;

    ///
    /// \brief Close the connection to the VRPN server
    /// \return DeviceInterface::Message::Success on success and
    /// DeviceInterface::Message::Failure on failure
    ///
    virtual DeviceInterface::Message closeDevice() override;

    ///
    /// \brief Set the type used to instantiate the VRPN device
    /// \detail Check DeviceType enumeration for more information
    /// \param t The new device type to use.
    ///
    void setDeviceType(const DeviceType t);

    ///
    /// \brief Get the phantom's device type
    /// \return The current device type used to instantiate the VRPN device
    ///
    const DeviceType &getDeviceType() const;

    ///
    /// \brief Set the url used to connect to the VRPN server
    /// \detail Example URLs: Phantom0@localhost, PHANToM2@10.0.0.13
    /// \param s The new device URL to use.
    ///
    void setDeviceURL(const std::string s);

    ///
    /// \brief Get the phantom's device URL
    /// \return The current device URL used to connect to the VRPN server
    ///
    const std::string &getDeviceURL() const;

    ///
    /// \brief Execute this module
    ///
    void exec() override;

protected:
    ///
    /// \brief Executes the mainloop of each of the VRPN interfaces to obtain data
    /// \detail If there is new data, the callback functions will be called
    ///
    virtual void processChanges();

private:
    ///
    /// \brief VRPN call back for button changed (pressed or released)
    /// \param userData Pointer to this to allow updating
    /// internal data
    /// \param b VRPN callback structure containing new button data
    ///
    static void VRPN_CALLBACK buttonChangeHandler(void *userData, const vrpn_BUTTONCB b);

    ///
    /// \brief VRPN call back for velocity data
    /// \param userData Pointer to this to allow updating
    /// internal data
    /// \param v VRPN callback structure containing new position and
    /// orientation data
    ///
    static void VRPN_CALLBACK velocityChangeHandler(void *userData,
                                                    const vrpn_TRACKERVELCB v);

    ///
    /// \brief VRPN call back for position and orientation data
    /// \param userData Pointer to this to allow updating
    /// internal data
    /// \param b VRPN callback structure containing new position and
    /// orientation data
    ///
    static void VRPN_CALLBACK trackerChangeHandler(void *userData, const vrpn_TRACKERCB b);

    ///
    /// \brief VRPN call back for position and orientation data
    /// \param userData Pointer to this to allow updating
    /// internal data
    /// \param b VRPN callback structure containing new position and
    /// orientation data
    ///
    static void VRPN_CALLBACK analogChangeHandler(void *userData, const vrpn_ANALOGCB a);

protected:
    std::string deviceURL;                              //!< Connection device URL

private:
    std::shared_ptr<vrpn_Button_Remote> vrpnButton;     //!< VRPN button interface
    std::shared_ptr<vrpn_Tracker_Remote> vrpnTracker;   //!< VRPN position/orientation interface
    std::shared_ptr<vrpn_Analog_Remote> vrpnAnalog;   //!< VRPN position/orientation interface
};

}

#endif // VRPNDEVICE_H
