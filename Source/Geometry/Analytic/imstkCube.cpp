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

#include "imstkCube.h"
#include "imstkLogger.h"

namespace imstk
{
void
Cube::print() const
{
    Geometry::print();
    LOG(INFO) << "Width: " << m_width;
}

double
Cube::getWidth(DataType type /* = DataType::PostTransform */)
{
    if (type == DataType::PostTransform)
    {
        this->updatePostTransformData();
        return m_widthPostTransform;
    }
    return m_width;
}

void
Cube::setWidth(const double w)
{
    if (w <= 0)
    {
        LOG(WARNING) << "Cube::setWidth error: width should be positive.";
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
Cube::applyTransform(const Mat4d& m)
{
    AnalyticalGeometry::applyTransform(m);
    /*Vec3d s = Vec3d(
        m_transform.block<3, 1>(0, 0).norm(),
        m_transform.block<3, 1>(0, 1).norm(),
        m_transform.block<3, 1>(0, 2).norm());*/
    const double s0 = m.block<3, 1>(0, 0).norm();
    this->setWidth(m_width * s0);
    this->postModified();
}

void
Cube::updatePostTransformData() const
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
Cube::computeBoundingBox(Vec3d& min, Vec3d& max, const double imstkNotUsed(paddingPercent))
{
    updatePostTransformData();

    const Mat3d r = Quatd::FromTwoVectors(Vec3d(0.0, 1.0, 0.0), m_orientationAxisPostTransform).toRotationMatrix();

    const Vec3d a = r.col(0) * m_widthPostTransform * 0.5;
    const Vec3d b = r.col(1) * m_widthPostTransform * 0.5;
    const Vec3d c = r.col(2) * m_widthPostTransform * 0.5;

    Vec3d pts[8];
    pts[0] = m_positionPostTransform + a + b + c;
    pts[1] = m_positionPostTransform + a + b - c;
    pts[2] = m_positionPostTransform + a - b + c;
    pts[3] = m_positionPostTransform + a - b - c;
    pts[4] = m_positionPostTransform - a + b + c;
    pts[5] = m_positionPostTransform - a + b - c;
    pts[6] = m_positionPostTransform - a - b + c;
    pts[7] = m_positionPostTransform - a - b - c;

    min = pts[0];
    max = pts[0];
    for (int i = 1; i < 8; i++)
    {
        min = min.cwiseMin(pts[i]);
        max = max.cwiseMax(pts[i]);
    }
}
} // imstk
