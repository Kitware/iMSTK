/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCapsule.h"
#include "imstkLogger.h"

namespace imstk
{
void
Capsule::print() const
{
    Geometry::print();
    LOG(INFO) << "Radius: " << m_radius;
    LOG(INFO) << "Length: " << m_length;
}

double
Capsule::getRadius(DataType type /* = DataType::PostTransform */)
{
    if (type == DataType::PostTransform)
    {
        this->updatePostTransformData();
        return m_radiusPostTransform;
    }
    return m_radius;
}

void
Capsule::setRadius(const double r)
{
    CHECK(r > 0) << "Capsule::setRadius error: radius should be positive.";
    m_radius = r;
    m_transformApplied = false;
    this->postModified();
}

double
Capsule::getLength(DataType type /* = DataType::PostTransform */)
{
    if (type == DataType::PostTransform)
    {
        this->updatePostTransformData();
        return m_lengthPostTransform;
    }
    return m_length;
}

void
Capsule::setLength(const double l)
{
    CHECK(l >= 0) << "Capsule::setLength error: length should be positive.";
    m_length = l;
    m_transformApplied = false;
    this->postModified();
}

double
Capsule::getFunctionValue(const Vec3d& x) const
{
    // Two lines points
    const Vec3d orientationAxes = m_orientationPostTransform.toRotationMatrix().col(1);
    const Vec3d a = m_positionPostTransform + 0.5 * orientationAxes * m_lengthPostTransform;
    const Vec3d b = 2.0 * m_positionPostTransform - a;

    const Vec3d  pa = x - a;
    const Vec3d  ba = b - a;
    const double h  = std::min(std::max(pa.dot(ba) / ba.dot(ba), 0.0), 1.0);
    return (pa - ba * h).norm() - m_radiusPostTransform;
}

void
Capsule::applyTransform(const Mat4d& m)
{
    AnalyticalGeometry::applyTransform(m);
    const double s = std::sqrt(Vec3d(
         m.block<3, 1>(0, 0).squaredNorm(),
         m.block<3, 1>(0, 1).squaredNorm(),
         m.block<3, 1>(0, 2).squaredNorm()).maxCoeff());
    this->setRadius(m_radius * s);
    this->setLength(m_length * s);
    this->postModified();
}

void
Capsule::updatePostTransformData() const
{
    if (m_transformApplied)
    {
        return;
    }
    AnalyticalGeometry::updatePostTransformData();
    const double s = getScaling().maxCoeff();
    m_radiusPostTransform = s * m_radius;
    m_lengthPostTransform = s * m_length;
    m_transformApplied    = true;
}

void
Capsule::computeBoundingBox(Vec3d& min, Vec3d& max, const double imstkNotUsed(paddingPercent))
{
    updatePostTransformData();

    const Vec3d orientationAxes = m_orientationPostTransform.toRotationMatrix().col(1);
    const Vec3d l  = m_lengthPostTransform * 0.5 * orientationAxes;
    const Vec3d p1 = m_positionPostTransform - l;
    const Vec3d p2 = m_positionPostTransform + l;

    min  = p1.cwiseMin(p2);
    max  = p1.cwiseMax(p2);
    min -= Vec3d(m_radiusPostTransform, m_radiusPostTransform, m_radiusPostTransform);
    max += Vec3d(m_radiusPostTransform, m_radiusPostTransform, m_radiusPostTransform);
}
} // namespace imstk