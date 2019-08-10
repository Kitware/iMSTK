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

#include "g3log/g3log.hpp"

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
Capsule::getVolume() const
{
    return PI * m_radius * m_radius * (m_length + 4.0 / 3.0 * m_radius);
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
    if (r <= 0)
    {
        LOG(FATAL) << "Capsule::setRadius error: radius should be positive.";
    }
    m_radius           = r;
    m_dataModified     = true;
    m_transformApplied = false;
}

double
Capsule::getLength(DataType type /* = DataType::PostTransform */)
{
    if (type == DataType::PostTransform)
    {
        this->updatePostTransformData();
        return m_radiusPostTransform;
    }
    return m_radius;
}

void
Capsule::setLength(const double l)
{
    if (l <= 0)
    {
        LOG(FATAL) << "Capsule::setHeight error: height should be positive.";
    }
    m_length           = l;
    m_dataModified     = true;
    m_transformApplied = false;
}

void
Capsule::applyScaling(const double s)
{
    this->setRadius(m_radius * s);
    this->setLength(m_length * s);
}

void
Capsule::updatePostTransformData()
{
    if (m_transformApplied)
    {
        return;
    }
    AnalyticalGeometry::updatePostTransformData();
    m_radiusPostTransform = m_scaling * m_radius;
    m_lengthPostTransform = m_scaling * m_length;
    m_transformApplied    = true;
}
} // imstk
