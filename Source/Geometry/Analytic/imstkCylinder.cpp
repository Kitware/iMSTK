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

#include "g3log/g3log.hpp"

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
Cylinder::getVolume() const
{
    return PI * m_radius * m_radius * m_length;
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

    m_radius           = r;
    m_dataModified     = true;
    m_transformApplied = false;
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
    m_length           = l;
    m_dataModified     = true;
    m_transformApplied = false;
}

void
Cylinder::applyScaling(const double s)
{
    this->setRadius(m_radius * s);
    this->setLength(m_length * s);
}

void
Cylinder::updatePostTransformData() const
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
