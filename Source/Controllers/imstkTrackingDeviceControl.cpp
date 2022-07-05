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

#include "imstkTrackingDeviceControl.h"
#include "imstkDeviceClient.h"
#include "imstkLogger.h"
#include "imstkMath.h"

namespace imstk
{
TrackingDeviceControl::TrackingDeviceControl(const std::string& name) :
    DeviceControl(name),
    m_translationOffset(Vec3d::Zero()),
    m_rotationOffset(Quatd::Identity())
{
}

bool
TrackingDeviceControl::updateTrackingData(const double dt)
{
    if (m_deviceClient == nullptr)
    {
        LOG(WARNING) << "warning: no controlling device set.";
        return false;
    }

    m_deviceClient->update();

    // Retrieve device info
    const Vec3d prevPos = m_currentPos;
    const Quatd prevOrientation = m_currentOrientation;

    m_currentPos = m_deviceClient->getPosition();
    m_currentOrientation     = m_deviceClient->getOrientation();
    m_currentVelocity        = m_deviceClient->getVelocity();
    m_currentAngularVelocity = m_deviceClient->getAngularVelocity();

    // Apply inverse if needed
    if (m_invertFlags & InvertFlag::transX)
    {
        m_currentPos[0]      = -m_currentPos[0];
        m_currentVelocity[0] = -m_currentVelocity[0];
    }
    if (m_invertFlags & InvertFlag::transY)
    {
        m_currentPos[1]      = -m_currentPos[1];
        m_currentVelocity[1] = -m_currentVelocity[1];
    }
    if (m_invertFlags & InvertFlag::transZ)
    {
        m_currentPos[2]      = -m_currentPos[2];
        m_currentVelocity[2] = -m_currentVelocity[2];
    }
    if (m_invertFlags & InvertFlag::rotX)
    {
        m_currentOrientation.y()    = -m_currentOrientation.y();
        m_currentOrientation.z()    = -m_currentOrientation.z();
        m_currentAngularVelocity[0] = -m_currentAngularVelocity[0];
    }
    if (m_invertFlags & InvertFlag::rotY)
    {
        m_currentOrientation.x()    = -m_currentOrientation.x();
        m_currentOrientation.z()    = -m_currentOrientation.z();
        m_currentAngularVelocity[1] = -m_currentAngularVelocity[1];
    }
    if (m_invertFlags & InvertFlag::rotZ)
    {
        m_currentOrientation.x()    = -m_currentOrientation.x();
        m_currentOrientation.y()    = -m_currentOrientation.y();
        m_currentAngularVelocity[2] = -m_currentAngularVelocity[2];
    }

    // Apply Offsets
    m_currentPos = m_rotationOffset * m_currentPos * m_scaling + m_translationOffset;
    m_currentOrientation = m_effectorRotationOffset * m_rotationOffset * m_currentOrientation;

    // With simulation substeps this may produce 0 deltas, but its fine
    // Another option is to divide velocity by number of substeps and then
    // maintain it for N substeps

    if (m_computeVelocity)
    {
        m_currentDisplacement = (m_currentPos - prevPos);
        m_currentVelocity     = m_currentDisplacement / dt;
    }
    if (m_computeAngularVelocity)
    {
        m_currentRotation = prevOrientation * m_currentOrientation.inverse();
        /* Rotd r = Rotd(m_currentRotation);
           r.angle() /= timestepInfo.m_dt;*/
        m_currentAngularVelocity = m_currentRotation.toRotationMatrix().eulerAngles(0, 1, 2) /= dt;
    }

    m_trackingDataUptoDate = true;
    return true;
}

void
TrackingDeviceControl::applyForces()
{
}

void
TrackingDeviceControl::setTrackerToOutOfDate()
{
    m_trackingDataUptoDate = false;
}

void
TrackingDeviceControl::setTrackerToUpToDate()
{
    m_trackingDataUptoDate = true;
}

bool
TrackingDeviceControl::isTrackerUpToDate() const
{
    return m_trackingDataUptoDate;
}

const imstk::Vec3d&
TrackingDeviceControl::getPosition() const
{
    return m_currentPos;
}

void
TrackingDeviceControl::setPosition(const Vec3d& pos)
{
    this->m_currentPos = pos;
}

const imstk::Quatd&
TrackingDeviceControl::getOrientation() const
{
    return m_currentOrientation;
}

void
TrackingDeviceControl::setOrientation(const Quatd& orientation)
{
    this->m_currentOrientation = orientation;
}

void
TrackingDeviceControl::setComputeVelocity(const bool computeVelocity)
{
    m_computeVelocity = computeVelocity;
}

bool
TrackingDeviceControl::getComputeVelocity() const
{
    return m_computeVelocity;
}

void
TrackingDeviceControl::setComputeAngularVelocity(const bool computeAngularVelocity)
{
    m_computeAngularVelocity = computeAngularVelocity;
}

bool
TrackingDeviceControl::getComputeAngularVelocity() const
{
    return m_computeAngularVelocity;
}

const imstk::Vec3d&
TrackingDeviceControl::getAngularVelocity() const
{
    return m_currentAngularVelocity;
}

void
TrackingDeviceControl::setAngularVelocity(const Vec3d& angularVelocity)
{
    m_currentAngularVelocity = angularVelocity;
}

const imstk::Vec3d&
TrackingDeviceControl::getVelocity() const
{
    return m_currentVelocity;
}

void
TrackingDeviceControl::setVelocity(const Vec3d& velocity)
{
    m_currentVelocity = velocity;
}

double
TrackingDeviceControl::getTranslationScaling() const
{
    return m_scaling;
}

void
TrackingDeviceControl::setTranslationScaling(const double scaling)
{
    m_scaling = scaling;
}

const Vec3d&
TrackingDeviceControl::getTranslationOffset() const
{
    return m_translationOffset;
}

void
TrackingDeviceControl::setTranslationOffset(const Vec3d& t)
{
    m_translationOffset = t;
}

const Quatd&
TrackingDeviceControl::getRotationOffset()
{
    return m_rotationOffset;
}

void
TrackingDeviceControl::setRotationOffset(const Quatd& r)
{
    m_rotationOffset = r;
}

const imstk::Quatd&
TrackingDeviceControl::getEffectorRotationOffset()
{
    return m_effectorRotationOffset;
}

void
TrackingDeviceControl::setEffectorRotationOffset(const Quatd& r)
{
    m_effectorRotationOffset = r;
}

unsigned char
TrackingDeviceControl::getInversionFlags()
{
    return m_invertFlags;
}

void
TrackingDeviceControl::setInversionFlags(const unsigned char f)
{
    m_invertFlags = f;
}
} // namespace imstk