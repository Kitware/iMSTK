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

#ifndef imstkDeviceClient_h
#define imstkDeviceClient_h

#include <map>

#include "imstkModule.h"
#include "imstkMath.h"

namespace imstk {

enum class DeviceType
{
    SPACE_EXPLORER_3DCONNEXION,
    NAVIGATOR_3DCONNEXION
};

///
/// \class DeviceClient
/// \brief Base class for any device client
///
class DeviceClient : public Module
{
public:

    virtual ~DeviceClient() {}

    // Accessors
    ///
    /// \brief Get/Set the type of the device
    ///
    const DeviceType& getType();
    void setType(const DeviceType& type);

    ///
    /// \brief Get/Set the device URL
    ///
    const std::string& getUrl();
    void setUrl(const std::string& url);

    ///
    /// \brief Get/Set what listeners to enable on the device: tracking, analogic, force, buttons.
    ///
    const bool& getTrackingEnabled() const;
    void setTrackingEnabled(const bool& status);
    const bool& getAnalogicEnabled() const;
    void setAnalogicEnabled(const bool& status);
    const bool& getButtonsEnabled() const;
    void setButtonsEnabled(const bool& status);
    const bool& getForceEnabled() const;
    void setForceEnabled(const bool& status);

    ///
    /// \brief Get the device position
    ///
    const Vec3d& getPosition() const;

    ///
    /// \brief Get the device velocity
    ///
    const Vec3d& getVelocity() const;

    ///
    /// \brief Get the device orientation
    ///
    const Quatd& getOrientation() const;

    ///
    /// \brief Get the status of the device buttons
    ///
    const std::map<size_t, bool>& getButtons() const;

    ///
    /// \brief Get the status of a device button
    ///
    bool getButton(size_t buttonId) const;

    ///
    /// \brief Get the device force
    ///
    const Vec3d& getForce() const;

    ///
    /// \brief Get the device torque
    ///
    const Vec3d& getTorque() const;

protected:

    DeviceClient(std::string name, std::string url, DeviceType type):
        Module(name),
        m_url(url),
        m_type(type)
    {}

    DeviceType m_type; //!< Device type
    std::string m_url; //!< Connection device URL

    bool m_trackingEnabled = true; //!< Tracking enabled if true
    bool m_analogicEnabled = true; //!< Analogic enabled if true
    bool m_buttonsEnabled = true;  //!< Buttons enabled if true
    bool m_forceEnabled = false;   //!< Force enabled if true

    Vec3d m_position = Vec3d::Zero();        //!< Position of end effector
    Vec3d m_velocity = Vec3d::Zero();        //!< Linear velocity of end effector
    Quatd m_orientation = Quatd::Identity(); //!< Orientation of the end effector
    std::map<size_t, bool> m_buttons;        //!< Buttons: true = pressed/false = not pressed
    Vec3d m_force = Vec3d::Zero();           //!< Force vector
    Vec3d m_torque = Vec3d::Zero();          //!< Torque vector
};
}

#endif // ifndef imstkDeviceClient_h
