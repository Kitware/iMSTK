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
    CHECK(r > 0) << "Cylinder::setRadius error: radius should be positive.";

    if (m_radius == r)
    {
        return;
    }

    m_radius = r;
    m_transformApplied = false;
    this->postEvent(Event(EventType::Modified));
}

void
Cylinder::setLength(const double l)
{
    if (l <= 0)
    {
        LOG(WARNING) << "Cylinder::setLength error: length should be positive.";
        return;
    }
    if (m_length == l)
    {
        return;
    }
    m_length = l;
    m_transformApplied = false;
    this->postEvent(Event(EventType::Modified));
}

void
Cylinder::applyTransform(const Mat4d& m)
{
    AnalyticalGeometry::applyTransform(m);
    /*const Vec3d s = Vec3d(
        m.block<3, 1>(0, 0).norm(),
        m.block<3, 1>(0, 1).norm(),
        m.block<3, 1>(0, 2).norm());*/
    const double s0 = m.block<3, 1>(0, 0).norm();
    this->setRadius(m_radius * s0);
    this->setLength(m_length * s0);
    this->modified();
}

void
Cylinder::updatePostTransformData() const
{
    if (m_transformApplied)
    {
        return;
    }
    AnalyticalGeometry::updatePostTransformData();
    const double s0 = m_transform.block<3, 1>(0, 0).norm();
    m_radiusPostTransform = s0 * m_radius;
    m_lengthPostTransform = s0 * m_length;
    m_transformApplied    = true;
}

void
Cylinder::computeBoundingBox(Vec3d& min, Vec3d& max, const double imstkNotUsed(paddingPercent))
{
    updatePostTransformData();

    const Vec3d d  = m_orientationAxisPostTransform * m_lengthPostTransform * 0.5;
    const Vec3d p1 = m_positionPostTransform - d;
    const Vec3d p2 = m_positionPostTransform + d;

    const Vec3d  a = p2 - p1;
    const double qSqrLength = a.dot(a);
    const Vec3d  e = m_radiusPostTransform * (Vec3d(1.0, 1.0, 1.0) - a.cwiseProduct(a).cwiseQuotient(Vec3d(qSqrLength, qSqrLength, qSqrLength))).cwiseSqrt();
    min = (p1 - e).cwiseMin(p2 - e);
    max = (p1 + e).cwiseMax(p2 + e);
}
} // imstk
