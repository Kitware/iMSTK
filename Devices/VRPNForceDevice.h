// This file is part of the iMSTK project.
//
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

#ifndef DEVICES_VRPNFORCEDEVICE_H
#define DEVICES_VRPNFORCEDEVICE_H

#include "Devices/VRPNDeviceClient.h"

class vrpn_ForceDevice_Remote;
typedef struct _vrpn_FORCECB vrpn_FORCECB;

namespace imstk {

///
/// \brief An interface for the Phantom Omni or Geomagic Touch using VRPN.
///     This is the main client code.
///
class VRPNForceDevice : public VRPNDeviceClient
{
public:
    ///
    /// \brief Constructor/Destructor
    ///
    VRPNForceDevice(std::string deviceURL);
    ~VRPNForceDevice();

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
    bool init() override;

    ///
    /// \brief Enable force feedback
    ///
    void setEnableForce(const bool &enable);

    ///
    /// \brief Get enabling force variable
    ///
    const bool &getEnableForce() const;

    ///
    /// \brief Set contact plane
    ///
    void setContactPlane(const Vec4f &plane);

    ///
    /// \brief Set contact plane
    ///
    void setContactPlane(const Vec3f &plane, const float d);

    ///
    /// \brief Get contact plane
    ///
    const Vec4f &getContactPlane() const;

    ///
    /// \brief Set damping coefficient
    ///
    void setDampingCoefficient(const double &coeff);

    ///
    /// \brief Get damping coefficient
    ///
    const double &getDampingCoefficient() const;

    ///
    /// \brief Set dynamic friction coefficient
    ///
    void setDynamicFriction(const double &coeff);

    ///
    /// \brief Get dynamic friction coefficient
    ///
    const double &getDynamicFriction() const;

    ///
    /// \brief Set spring coefficient
    ///
    void setSpringCoefficient(const double &coeff);

    ///
    /// \brief Get spring coefficient
    ///
    const double &getSpringCoefficient() const;

    ///
    /// \brief Set static friction coefficient
    ///
    void setStaticFriction(const double &coeff);

    ///
    /// \brief Get static friction coefficient
    ///
    const double &getStaticFriction() const;

protected:
    ///
    /// \brief Executes the mainloop of each of the VRPN interfaces to obtain data
    /// \detail If there is new data, the callback functions will be called
    ///
    void processChanges() override;

private:
    ///
    /// \brief VRPN call back for force data
    /// \param userData Pointer to this to allow updating
    /// internal data
    /// \param f VRPN callback structure containing new force data
    ///
    static void VRPN_CALLBACK forceChangeHandler(void *userData, const vrpn_FORCECB f);

private:
    bool enableForce;           //!< Enable force feedback
    Vec4f contactPlane;   //!< Contact plane as [normal,d] where dot(n,x) = d.
    double dampingCoefficient;  //!< Damping damping coefficient.
    double dynamicFriction;     //!< Dynamic friction coefficient.
    double springCoefficient;   //!< Spring constant.
    double staticFriction;      //!< Static friction coefficient.
    std::shared_ptr<vrpn_ForceDevice_Remote> vrpnForce; //!< VRPN force interface

};

}

#endif
