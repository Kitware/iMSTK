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

#include <memory>

#include "imstkCamera.h"
#include "imstkCameraController.h"

#include <g3log/g3log.hpp>

namespace imstk
{
const Vec3d&
Camera::getPosition() const
{
    return m_position;
}

void
Camera::setPosition(const Vec3d& p)
{
    m_position = p;
}

void
Camera::setPosition(const double& x,
                    const double& y,
                    const double& z)
{
    m_position = Vec3d(x,y,z);
}

const Vec3d&
Camera::getFocalPoint() const
{
    return m_focalPoint;
}

void
Camera::setFocalPoint(const Vec3d& p)
{
    m_focalPoint = p;
}

void
Camera::setFocalPoint(const double& x,
                      const double& y,
                      const double& z)
{
    m_focalPoint = Vec3d(x,y,z);
}

const Vec3d&
Camera::getViewUp() const
{
    return m_viewUp;
}

void
Camera::setViewUp(const Vec3d& v)
{
    m_viewUp = v;
}

void
Camera::setViewUp(const double& x,
                  const double& y,
                  const double& z)
{
    m_viewUp = Vec3d(x,y,z).normalized();
}

const double& Camera::getViewAngle() const
{
    return m_viewAngle;
}

void
Camera::setViewAngle(const double& angle)
{
    m_viewAngle = angle;
}

std::shared_ptr<CameraController>
Camera::getController() const
{
    return m_controller;
}

std::shared_ptr<CameraController>
Camera::setupController(std::shared_ptr<DeviceClient> deviceClient)
{
    if(m_controller == nullptr)
    {
        m_controller = std::make_shared<CameraController>(*this, deviceClient);
    }
    else
    {
        m_controller->setDeviceClient(deviceClient);
    }
    return m_controller;
}
} // imstk
