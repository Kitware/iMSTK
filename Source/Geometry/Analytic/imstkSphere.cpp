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

#include "imstkSphere.h"
#include "imstkLogger.h"

namespace imstk
{
void
Sphere::print() const
{
    AnalyticalGeometry::print();
    LOG(INFO) << "Radius: " << m_radius;
}

double
Sphere::getRadius(DataType type /* = DataType::PostTransform */)
{
    if (type == DataType::PostTransform)
    {
        this->updatePostTransformData();
        return m_radiusPostTransform;
    }
    return m_radius;
}

void
Sphere::setRadius(const double r)
{
    if (r <= 0)
    {
        LOG(WARNING) << "Sphere::setRadius error: radius should be positive.";
        return;
    }
    if (std::abs(m_radius - r) < 1e-20)
    {
        return;
    }
    m_radius = r;
    m_transformApplied = false;
    this->postModified();
}

void
Sphere::computeBoundingBox(Vec3d& lowerCorner, Vec3d& upperCorner, const double paddingPercent)
{
    updatePostTransformData();
    const Vec3d span = Vec3d(1, 1, 1) * m_radiusPostTransform;
    lowerCorner = m_positionPostTransform - span;
    upperCorner = m_positionPostTransform + span;
    if (paddingPercent > 0.0)
    {
        const Vec3d range = upperCorner - lowerCorner;
        lowerCorner = lowerCorner - range * (paddingPercent / 100.0);
        upperCorner = upperCorner + range * (paddingPercent / 100.0);
    }
}

void
Sphere::applyTransform(const Mat4d& m)
{
    AnalyticalGeometry::applyTransform(m);
    /*const Vec3d s = Vec3d(
        m.block<3, 1>(0, 0).norm(),
        m.block<3, 1>(0, 1).norm(),
        m.block<3, 1>(0, 2).norm());*/
    const double s0 = m.block<3, 1>(0, 0).norm();
    this->setRadius(m_radius * s0);
}

void
Sphere::updatePostTransformData() const
{
    if (m_transformApplied)
    {
        return;
    }
    AnalyticalGeometry::updatePostTransformData();
    const double s0 = m_transform.block<3, 1>(0, 0).norm();
    m_radiusPostTransform = s0 * m_radius;
    m_transformApplied    = true;
}
} // imstk
