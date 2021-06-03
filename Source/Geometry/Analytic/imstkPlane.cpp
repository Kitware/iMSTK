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
#include "imstkLogger.h"

namespace imstk
{
Vec3d
Plane::getNormal(DataType type /* = DataType::PostTransform */)
{
    if (type == DataType::PostTransform)
    {
        this->updatePostTransformData();
        return m_normalPostTransform;
    }
    return m_normal;
}

void
Plane::setNormal(const Vec3d n)
{
    // A normal with no direction would destroy the basis
    // of the transform
    if (m_normal == n || n.norm() == 0.0)
    {
        return;
    }
    m_normal = n.normalized();
    m_transformApplied = false;
    this->postModified();
}

void
Plane::setNormal(const double x, const double y, const double z)
{
    this->setNormal(Vec3d(x, y, z));
}

double
Plane::getWidth()
{
    return m_width;
}

void
Plane::setWidth(const double w)
{
    if (m_width == w || m_width <= 0.0)
    {
        return;
    }
    m_width = w;
    this->postModified();
}

void
Plane::applyTransform(const Mat4d& m)
{
    AnalyticalGeometry::applyTransform(m);
    this->postModified();
}

void
Plane::updatePostTransformData() const
{
    if (m_transformApplied)
    {
        return;
    }
    AnalyticalGeometry::updatePostTransformData();
    m_normalPostTransform = m_orientation._transformVector(m_normal);
    m_transformApplied    = true;
}

void
Plane::computeBoundingBox(Vec3d& min, Vec3d& max, const double imstkNotUsed(paddingPercent))
{
    updatePostTransformData();

    Vec3d p1 = m_position + Vec3d(0.5, 0.0, 0.5);
    Vec3d p2 = m_position + Vec3d(0.5, 0.0, -0.5);
    Vec3d p3 = m_position + Vec3d(-0.5, 0.0, 0.5);
    Vec3d p4 = m_position + Vec3d(-0.5, 0.0, -0.5);

    p1 = (m_transform * Vec4d(p1[0], p1[1], p1[2], 1.0)).head<3>();
    p2 = (m_transform * Vec4d(p2[0], p2[1], p2[2], 1.0)).head<3>();
    p3 = (m_transform * Vec4d(p3[0], p3[1], p3[2], 1.0)).head<3>();
    p4 = (m_transform * Vec4d(p4[0], p4[1], p4[2], 1.0)).head<3>();

    min = p1.cwiseMin(p2);
    min = min.cwiseMin(p3);
    min = min.cwiseMin(p4);

    max = p1.cwiseMax(p2);
    max = max.cwiseMax(p3);
    max = max.cwiseMax(p4);
}
} // imstk
