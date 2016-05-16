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

#include <utility>

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
    Vec3d p = m_deviceClient->getPosition();
    Quatd r = m_deviceClient->getOrientation();

    // Apply inverse if needed
    if(m_invertFlags & InvertFlag::transX) p[0] = -p[0];
    if(m_invertFlags & InvertFlag::transY) p[1] = -p[1];
    if(m_invertFlags & InvertFlag::transZ) p[2] = -p[2];
    if(m_invertFlags & InvertFlag::rotX) std::swap(r.y(), r.z());
    if(m_invertFlags & InvertFlag::rotY) std::swap(r.x(), r.z());
    if(m_invertFlags & InvertFlag::rotZ) std::swap(r.x(), r.y());

    // Apply Offsets
    p = m_rotationOffset * p * m_scaling + m_translationOffset;
    r *= m_rotationOffset;

    // Set camera info
    m_camera.setPosition(p);
    m_camera.setFocalPoint((r*FORWARD_VECTOR)+p);
    m_camera.setViewUp(r*UP_VECTOR);
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

unsigned char
CameraController::getInversionFlags()
{
    return m_invertFlags;
}

void
CameraController::setInversionFlags(unsigned char f)
{
    m_invertFlags = f;
}
}
