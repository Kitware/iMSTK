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

#include <map>

#include "imstkDeviceClient.h"
#include "imstkModule.h"

#include <memory>

#include <vrpn_Configure.h>
#include <vrpn_Analog.h>

namespace imstk
{
///
/// \class VRPNDeviceClient
/// \brief Subclass of DeviceClient using VRPN
///
class VRPNArduinoDeviceClient : public DeviceClient, public Module
{
public:

    ///
    /// \brief Constructor
    ///
    VRPNArduinoDeviceClient(std::string deviceName, std::string ip) :
        DeviceClient(deviceName, ip),
        Module(deviceName + "@" + ip)
    {}

    ///
    /// \brief Destructor
    ///
    virtual ~VRPNArduinoDeviceClient()
    {}

    Vec3d& getYPR() {return m_ypr;}

    Vec3d& getAcceleration(){return m_accel;}
    float getRoll(){return m_roll;}

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
    static void VRPN_CALLBACK analogChangeHandler(void *userData, const _vrpn_ANALOGCB a);

    std::shared_ptr<vrpn_Analog_Remote> m_vrpnAnalog;           //!< VRPN position/orientation interface
    Vec3d m_ypr = Vec3d::Zero();
    Vec3d m_accel = Vec3d::Zero();
    float m_roll = 0;
};
}

