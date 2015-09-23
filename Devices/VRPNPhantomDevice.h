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

class vrpn_ForceDevice_Remote;
typedef struct _vrpn_FORCECB vrpn_FORCECB;

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

    ///
    /// \brief Enable force feedback
    ///
    void setEnableForce(const bool &enable)
    {
        this->enableForce = enable;
    }

    ///
    /// \brief Get enabling force variable
    ///
    const bool &getEnableForce() const
    {
        return this->enableForce;
    }

    ///
    /// \brief Set contact plane
    ///
    void setContactPlane(const core::Vec4f &plane)
    {
        this->contactPlane = plane;
    }

    ///
    /// \brief Get contact plane
    ///
    const core::Vec4f &getContactPlane() const
    {
        return this->contactPlane;
    }

    ///
    /// \brief Set damping coefficient
    ///
    void setDampingCoefficient(const double &coeff)
    {
        this->dampingCoefficient = coeff;
    }

    ///
    /// \brief Get damping coefficient
    ///
    const double &getDampingCoefficient() const
    {
        return this->dampingCoefficient;
    }

    ///
    /// \brief Set dynamic friction coefficient
    ///
    void setDynamicFriction(const double &coeff)
    {
        this->dynamicFriction = coeff;
    }

    ///
    /// \brief Get dynamic friction coefficient
    ///
    const double &getDynamicFriction() const
    {
        return this->dynamicFriction;
    }

    ///
    /// \brief Set spring coefficient
    ///
    void setSpringCoefficient(const double &coeff)
    {
        this->springCoefficient = coeff;
    }

    ///
    /// \brief Get spring coefficient
    ///
    const double &getSpringCoefficient() const
    {
        return this->springCoefficient;
    }

    ///
    /// \brief Set static friction coefficient
    ///
    void setStaticFriction(const double &coeff)
    {
        this->staticFriction = coeff;
    }

    ///
    /// \brief Get static friction coefficient
    ///
    const double &getStaticFriction() const
    {
        return this->staticFriction;
    }

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
    core::Vec4f contactPlane;   //!< Contact plane as [normal,d] where dot(n,x) = d.
    double dampingCoefficient;  //!< Damping damping coefficient.
    double dynamicFriction;     //!< Dynamic friction coefficient.
    double springCoefficient;   //!< Spring constant.
    double staticFriction;      //!< Static friction coefficient.
    std::shared_ptr<vrpn_ForceDevice_Remote> vrpnForce; //!< VRPN force interface

};

#endif
