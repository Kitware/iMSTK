/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkAnalyticalGeometry.h"
#include "imstkLogger.h"

namespace imstk
{
AnalyticalGeometry::AnalyticalGeometry() : ImplicitGeometry(),
    m_position(Vec3d::Zero()), m_positionPostTransform(Vec3d::Zero()),
    m_orientation(Quatd::Identity()), m_orientationPostTransform(Quatd::Identity())
{
}

void
AnalyticalGeometry::print() const
{
    LOG(INFO) << "Position: (" <<
        m_position.x() << ", " <<
        m_position.y() << ", " <<
        m_position.z() << ")";
    LOG(INFO) << "Orientation: (" <<
        m_orientation.x() << ", " <<
        m_orientation.y() << ", " <<
        m_orientation.z() << ", " <<
        m_orientation.w();
}

Vec3d
AnalyticalGeometry::getPosition(DataType type /* = DataType::PostTransform */)
{
    if (type == DataType::PostTransform)
    {
        this->updatePostTransformData();
        return m_positionPostTransform;
    }
    return m_position;
}

void
AnalyticalGeometry::setPosition(const Vec3d p)
{
    if (m_position == p)
    {
        return;
    }

    m_position = p;
    m_transformApplied = false;
    this->postModified();
}

void
AnalyticalGeometry::setPosition(const double x, const double y, const double z)
{
    this->setPosition(Vec3d(x, y, z));
}

Quatd
AnalyticalGeometry::getOrientation(DataType type)
{
    if (type == DataType::PostTransform)
    {
        this->updatePostTransformData();
        return m_orientationPostTransform;
    }
    return m_orientation;
}

void
AnalyticalGeometry::setOrientation(const Quatd r)
{
    // Two quats can represent the same rotation, check coeffs
    // for true difference
    if (m_orientation.coeffs() == r.coeffs())
    {
        return;
    }

    m_orientation      = r;
    m_transformApplied = false;
    this->postModified();
}

void
AnalyticalGeometry::applyTransform(const Mat4d& m)
{
    this->setPosition((m * Vec4d(m_position[0], m_position[1], m_position[2], 1.0)).head<3>());
    this->setOrientation((m_orientation * Quatd(m.block<3, 3>(0, 0))).normalized());
}

void
AnalyticalGeometry::updatePostTransformData() const
{
    m_positionPostTransform    = (m_transform * Vec4d(m_position[0], m_position[1], m_position[2], 1.0)).head<3>();
    m_orientationPostTransform = (Quatd(getRotation()) * m_orientation).normalized();
}
} // namespace imstk