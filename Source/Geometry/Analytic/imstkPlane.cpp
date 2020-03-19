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

#include "imstkPlane.h"

namespace imstk
{
void
Plane::print() const
{
    AnalyticalGeometry::print();
    LOG(INFO) << "Width: " << m_width;
}

double
Plane::getVolume() const
{
    return 0.0;
}

Vec3d
Plane::getNormal(DataType type /* = DataType::PostTransform */)
{
    return this->getOrientationAxis(type);
}

void
Plane::setNormal(const Vec3d n)
{
    this->setOrientationAxis(n);
}

void
Plane::setNormal(double x, double y, double z)
{
    this->setNormal(Vec3d(x, y, z));
}

double
Plane::getWidth(DataType type /* = DataType::PostTransform */)
{
    if (type == DataType::PostTransform)
    {
        this->updatePostTransformData();
        return m_widthPostTransform;
    }
    return m_width;
}

void
Plane::setWidth(const double w)
{
    if (w <= 0)
    {
        LOG(WARNING) << "Plane::setWidth error: width should be positive.";
        return;
    }
    if (m_width == w)
    {
        return;
    }
    m_width = w;
    m_dataModified     = true;
    m_transformApplied = false;
}

void
Plane::applyScaling(const double s)
{
    this->setWidth(m_width * s);
}

void
Plane::updatePostTransformData() const
{
    if (m_transformApplied)
    {
        return;
    }
    AnalyticalGeometry::updatePostTransformData();
    m_widthPostTransform = m_scaling * m_width;
    m_transformApplied   = true;
}
} // imstk
