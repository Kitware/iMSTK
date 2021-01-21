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
    this->postEvent(Event(EventType::Modified));
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
    CHECK(l > 0) << "Capsule::setHeight error: height should be positive.";

    m_length = l;
    m_transformApplied = false;
    this->postEvent(Event(EventType::Modified));
}

double
Capsule::getFunctionValue(const Vec3d& x) const
{
    // Two lines points
    const Vec3d a = m_positionPostTransform + 0.5 * m_orientationAxisPostTransform * m_lengthPostTransform;
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
    /* const Vec3d s = Vec3d(
         m.block<3, 1>(0, 0).norm(),
         m.block<3, 1>(0, 1).norm(),
         m.block<3, 1>(0, 2).norm());*/
    const double s0 = m.block<3, 1>(0, 0).norm();
    this->setRadius(m_radius * s0);
    this->setLength(m_length * s0);
    this->modified();
}

void
Capsule::updatePostTransformData() const
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
} // imstk
