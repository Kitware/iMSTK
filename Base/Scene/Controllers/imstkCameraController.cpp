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

#include "imstkCameraController.h"

#include "imstkCamera.h"
#include "imstkDeviceClient.h"

#include <g3log/g3log.hpp>

namespace imstk {

void
CameraController::initModule()
{

}

void
CameraController::runModule()
{
    if (m_deviceClient == nullptr)
    {
        LOG(WARNING) << "CameraController::runModule warning: no controlling device set.";
        return;
    }

    // Retrieve device info
    Vec3d devicePos = m_translationOffset + m_deviceClient->getPosition();
    Quatd deviceRot = m_rotationOffset * m_deviceClient->getOrientation();

    // Set camera info
    m_camera.setPosition(devicePos*m_scaling);
    m_camera.setFocalPoint((deviceRot*FORWARD_VECTOR)+(devicePos*m_scaling));
    m_camera.setViewUp(deviceRot*DOWN_VECTOR);
}

void
CameraController::cleanUpModule()
{

}

std::shared_ptr<DeviceClient>
CameraController::getDeviceClient() const
{
    return m_deviceClient;
}

void
CameraController::setDeviceClient(std::shared_ptr<DeviceClient> deviceClient)
{
    m_deviceClient = deviceClient;
}

double
CameraController::getTranslationScaling() const
{
    return m_scaling;
}

void
CameraController::setTranslationScaling(double scaling)
{
    m_scaling = scaling;
}

const Vec3d&
CameraController::getTranslationOffset() const
{
    return m_translationOffset;
}

void
CameraController::setTranslationOffset(const Vec3d& t)
{
    m_translationOffset = t;
}

const Quatd&
CameraController::getRotationOffset()
{
    return m_rotationOffset;
}

void
CameraController::setRotationOffset(const Quatd& r)
{
    m_rotationOffset = r;
}
}
