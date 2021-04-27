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
void
Plane::print() const
{
    AnalyticalGeometry::print();
    LOG(INFO) << "Width: " << m_width;
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
Plane::setNormal(const double x, const double y, const double z)
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
        LOG(WARNING) << "error: width is negative.";
        return;
    }
    if (m_width == w)
    {
        return;
    }
    m_width = w;
    m_transformApplied = false;
    this->postModified();
}

void
Plane::applyTransform(const Mat4d& m)
{
    AnalyticalGeometry::applyTransform(m);
    /* const Vec3d s = Vec3d(
         m_transform.block<3, 1>(0, 0).norm(),
         m_transform.block<3, 1>(0, 1).norm(),
         m_transform.block<3, 1>(0, 2).norm());*/
    const double s0 = m.block<3, 1>(0, 0).norm();
    this->setWidth(m_width * s0);
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
    const double s0 = m_transform.block<3, 1>(0, 0).norm();
    m_widthPostTransform = s0 * m_width;
    m_transformApplied   = true;
}

void
Plane::computeBoundingBox(Vec3d& min, Vec3d& max, const double imstkNotUsed(paddingPercent))
{
    updatePostTransformData();
    const Quatd r = Quatd::FromTwoVectors(Vec3d(0.0, 1.0, 0.0), m_orientationAxisPostTransform);
    const Vec3d i = r._transformVector(Vec3d(1.0, 0.0, 0.0));
    const Vec3d j = r._transformVector(Vec3d(0.0, 0.0, 1.0));

    const Vec3d p1 = m_positionPostTransform + m_widthPostTransform * (i + j);
    const Vec3d p2 = m_positionPostTransform + m_widthPostTransform * (i - j);
    const Vec3d p3 = m_positionPostTransform + m_widthPostTransform * (-i + j);
    const Vec3d p4 = m_positionPostTransform + m_widthPostTransform * (-i - j);

    min = p1.cwiseMin(p2);
    min = min.cwiseMin(p3);
    min = min.cwiseMin(p4);

    max = p1.cwiseMax(p2);
    max = max.cwiseMax(p3);
    max = max.cwiseMax(p4);
}
} // imstk
