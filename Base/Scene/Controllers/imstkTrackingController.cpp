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

#include "imstkTrackingController.h"

#include <utility>

#include <g3log/g3log.hpp>

namespace imstk {

bool
TrackingController::computeTrackingData(Vec3d& p, Quatd& r)
{
    if (m_deviceClient == nullptr)
    {
        LOG(WARNING) << "TrackingController::getTrackingData warning: no controlling device set.";
        return false;
    }

    // Retrieve device info
    p = m_deviceClient->getPosition();
    r = m_deviceClient->getOrientation();

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

    return true;
}

std::shared_ptr<DeviceClient>
TrackingController::getDeviceClient() const
{
    return m_deviceClient;
}

void
TrackingController::setDeviceClient(std::shared_ptr<DeviceClient> deviceClient)
{
    m_deviceClient = deviceClient;
}

double
TrackingController::getTranslationScaling() const
{
    return m_scaling;
}

void
TrackingController::setTranslationScaling(double scaling)
{
    m_scaling = scaling;
}

const Vec3d&
TrackingController::getTranslationOffset() const
{
    return m_translationOffset;
}

void
TrackingController::setTranslationOffset(const Vec3d& t)
{
    m_translationOffset = t;
}

const Quatd&
TrackingController::getRotationOffset()
{
    return m_rotationOffset;
}

void
TrackingController::setRotationOffset(const Quatd& r)
{
    m_rotationOffset = r;
}

unsigned char
TrackingController::getInversionFlags()
{
    return m_invertFlags;
}

void
TrackingController::setInversionFlags(unsigned char f)
{
    m_invertFlags = f;
}
}
