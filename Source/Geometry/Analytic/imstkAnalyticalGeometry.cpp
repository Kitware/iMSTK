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

#include "imstkAnalyticalGeometry.h"

namespace imstk
{
void
AnalyticalGeometry::print() const
{
    LOG(INFO) << "Position: (" << m_position.x() << ", " << m_position.y() << ", " << m_position.z() << ")";
    LOG(INFO) << "Orientation Axis: (" << m_orientationAxis.x() << ", " << m_orientationAxis.y() << ", " << m_orientationAxis.z() << ")";
}

Vec3d
AnalyticalGeometry::getPosition(DataType type /* = DataType::PostTransform */)
{
    if (type == DataType::PostTransform)
    {
        this->updatePostTransformData();
        return m_positionPostTransform;
    }
    return m_position;
}

void
AnalyticalGeometry::setPosition(const Vec3d p)
{
    if (m_position == p)
    {
        return;
    }

    m_position         = p;
    m_dataModified     = true;
    m_transformApplied = false;
}

void
AnalyticalGeometry::setPosition(const double x, const double y, const double z)
{
    this->setPosition(Vec3d(x, y, z));
}

Vec3d
AnalyticalGeometry::getOrientationAxis(DataType type /* = DataType::PostTransform */)
{
    if (type == DataType::PostTransform)
    {
        this->updatePostTransformData();
        return m_orientationAxisPostTransform;
    }
    return m_orientationAxis;
}

void
AnalyticalGeometry::setOrientationAxis(const Vec3d orientation)
{
    if (orientation == Vec3d::Zero())
    {
        LOG(WARNING) << "AnalyticalGeometry::setOrientationAxis error: "
                     << "orientation can not be defined by a null vector.";
        return;
    }
    if (m_orientationAxis == orientation)
    {
        return;
    }
    m_orientationAxis  = orientation.normalized();
    m_dataModified     = true;
    m_transformApplied = false;
}

void
AnalyticalGeometry::applyTranslation(const Vec3d t)
{
    this->setPosition(m_position + t);
}

void
AnalyticalGeometry::applyRotation(const Mat3d r)
{
    this->setOrientationAxis(r * m_orientationAxis);
}

void
AnalyticalGeometry::updatePostTransformData()
{
    m_orientationAxisPostTransform = m_transform.rotation() * m_orientationAxis;
    m_orientationAxisPostTransform.normalize();
    m_positionPostTransform = m_position + m_transform.translation();
}
} // imstk
