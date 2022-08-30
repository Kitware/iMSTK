/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkTrackingDeviceControl.h"
#include "imstkDeviceClient.h"
#include "imstkLogger.h"
#include "imstkMath.h"

#include <cmath>

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

    // Apply scaling
    m_currentVelocity = m_currentVelocity * m_scaling;

    // With simulation substeps this may produce 0 deltas, but its fine
    // Another option is to divide velocity by number of substeps and then
    // maintain it for N substeps
    // Note: This velocity will not be as accurate as the one returned by the
    // haptic device, since the haptic devices runs on a seperate thread at a
    // higher rate.
    if (m_computeVelocity)
    {
        m_currentDisplacement = (m_currentPos - prevPos);
        m_currentVelocity     = m_currentDisplacement / dt;
    }
    if (m_computeAngularVelocity)
    {
        // Get axis of rotation in current configuration
        Rotd currentR = Rotd(m_currentOrientation);

        // Arbitrary normalized basis
        Vec3d basis = { 1.0, 0.0, 0.0 };

        // Vectors created by rotating basis using orientations
        Vec3d vec1 = m_currentOrientation.normalized().toRotationMatrix() * basis;
        Vec3d vec2 = prevOrientation.normalized().toRotationMatrix() * basis;

        // Angal between the two vectors after rotation, divided by timestep to get rate
        auto angle = std::acos(std::min(1.0, std::max(-1.0, vec1.dot(vec2)))) / dt;
        // Assume small change in rotation axis
        m_currentAngularVelocity = (angle * currentR.axis());
    }

    return true;
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