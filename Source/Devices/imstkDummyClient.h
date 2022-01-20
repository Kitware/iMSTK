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

#include <unordered_map>

namespace imstk
{
///
/// \class DummyClient
///
/// \brief Allows setting the pose of the device from external caller without a
///  real device connected
///
class DummyClient : public DeviceClient
{
public:
    DummyClient(const std::string& name) : DeviceClient(name, "localhost") {}
    ~DummyClient() override = default;

protected:
    ///
    /// \brief Initialize the phantom omni device
    ///
    void init(const unsigned int numButtons = 0)
    {
        for (unsigned int i = 0; i < numButtons; i++)
        {
            m_buttons[i] = false;
        }
    }

    void run()     = delete;
    void cleanUp() = delete;

public:
    ///
    /// \brief Set position
    ///
    void setPosition(const Vec3d& pos) { m_position = pos; }

    ///
    /// \brief Set velocity
    ///
    void setVelocity(const Vec3d& vel) { m_velocity = vel; }

    ///
    /// \brief Set orientation
    ///
    void setOrientation(const Quatd& orient) { m_orientation = orient; }

    ///
    /// \brief Set orientation from 4x4 transform
    ///
    void setOrientation(double* transform)
    {
        m_orientation = (Eigen::Affine3d(Eigen::Matrix4d(transform))).rotation();
    }

    ///
    /// \brief Set the button status if it exists
    ///
    void setButton(const unsigned int buttonId, const bool buttonStatus);

protected:
    std::unordered_map<int, bool> m_buttons;
};
}
