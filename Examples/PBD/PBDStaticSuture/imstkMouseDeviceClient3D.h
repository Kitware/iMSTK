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

#include "imstkMouseDeviceClient.h"

namespace imstk
{
///
/// \class MouseDeviceClient3D
///
/// \brief This device maps the mouse device into 3d coordinates
/// It does this with a single user transform. It's possible we
/// could add a user transform to the base DeviceClient (replacing
/// offset translation, offset orientation, etc)
/// This also has a convienence function map which maps one component
/// to another should you want mouse X -> world Z, use map(0, 2)
///
class MouseDeviceClient3D : public DeviceClient
{
public:
    MouseDeviceClient3D(std::shared_ptr<MouseDeviceClient> mouseClient) :
        DeviceClient("MouseDeviceClient3D", ""), m_mouseDeviceClient(mouseClient)
    {
        // Subscribe to the device clients events
        connect(m_mouseDeviceClient, MouseDeviceClient::mouseMove, this, &MouseDeviceClient3D::mouseMoved);
    }

    ///
    /// \brief Get/Set the transform from 2d->3d position
    ///
    void setTransform(const Mat4d& transform) { m_transform = transform; }
    Mat4d& getTransform() { return m_transform; }

    void setOrientation(const Quatd& orientation)
    {
        m_orientation = orientation;
    }

    ///
    /// \brief Map one component of 2d position to another of the 3d position produced
    /// ex: map(0, 2) -> the 2d x coordinate mapped to 3d z coordinate
    ///
    void map(int comp1, int comp2)
    {
        // Zero out the row of comp1 and comp2
        m_transform.block<1, 3>(comp1, 0) = Vec3d(0.0, 0.0, 0.0);
        m_transform.block<1, 3>(comp2, 0) = Vec3d(0.0, 0.0, 0.0);
        // Write a 1 such that the value at comp1 will be summed into resulting value in comp2
        m_transform.block<1, 3>(comp2, 0)[comp1] = 1.0;
    }

private:
    void mouseMoved(MouseEvent*)
    {
        const Vec2d& pos2d = m_mouseDeviceClient->getPos();
        m_position = (m_transform * Vec4d(pos2d[0], pos2d[1], 0.0, 1.0)).head<3>();
    }

private:
    std::shared_ptr<MouseDeviceClient> m_mouseDeviceClient = nullptr;
    Mat4d m_transform = Mat4d::Identity();
};
}; // namespace imstk
