/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCylinder.h"
#include "imstkLogger.h"

namespace imstk
{
void
Cylinder::print() const
{
    AnalyticalGeometry::print();
    LOG(INFO) << "Radius: " << m_radius;
    LOG(INFO) << "Length: " << m_length;
}

double
Cylinder::getRadius(DataType type /* = DataType::PostTransform */)
{
    if (type == DataType::PostTransform)
    {
        this->updatePostTransformData();
        return m_radiusPostTransform;
    }
    return m_radius;
}

double
Cylinder::getLength(DataType type /* = DataType::PostTransform */)
{
    if (type == DataType::PostTransform)
    {
        this->updatePostTransformData();
        return m_lengthPostTransform;
    }
    return m_length;
}

void
Cylinder::setRadius(const double r)
{
    CHECK(r > 0) << "error: radius should be positive.";

    if (m_radius == r)
    {
        return;
    }

    m_radius = r;
    m_transformApplied = false;
    this->postModified();
}

void
Cylinder::setLength(const double l)
{
    if (l <= 0)
    {
        LOG(WARNING) << "error: length is negative.";
        return;
    }
    if (m_length == l)
    {
        return;
    }
    m_length = l;
    m_transformApplied = false;
    this->postModified();
}

void
Cylinder::applyTransform(const Mat4d& m)
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
Cylinder::updatePostTransformData() const
{
    if (m_transformApplied)
    {
        return;
    }
    AnalyticalGeometry::updatePostTransformData();
    const double s = m_transform.block<3, 1>(0, 0).norm();
    m_radiusPostTransform = s * m_radius;
    m_lengthPostTransform = s * m_length;
    m_transformApplied    = true;
}

void
Cylinder::computeBoundingBox(Vec3d& min, Vec3d& max, const double imstkNotUsed(paddingPercent))
{
    updatePostTransformData();

    const Vec3d orientationAxes = m_orientationPostTransform.toRotationMatrix().col(1);
    const Vec3d d  = orientationAxes * m_lengthPostTransform * 0.5;
    const Vec3d p1 = m_positionPostTransform - d;
    const Vec3d p2 = m_positionPostTransform + d;

    const Vec3d  a = p2 - p1;
    const double qSqrLength = a.dot(a);
    const Vec3d  e = m_radiusPostTransform * (Vec3d(1.0, 1.0, 1.0) - a.cwiseProduct(a).cwiseQuotient(Vec3d(qSqrLength, qSqrLength, qSqrLength))).cwiseSqrt();
    min = (p1 - e).cwiseMin(p2 - e);
    max = (p1 + e).cwiseMax(p2 + e);
}
} // namespace imstk