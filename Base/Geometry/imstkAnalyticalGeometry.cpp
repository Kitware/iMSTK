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

namespace imstk {
void
AnalyticalGeometry::translate(const Vec3d& t)
{
    m_position += t;
}

void
AnalyticalGeometry::translate(const double& x,
                              const double& y,
                              const double& z)
{
    this->translate(Vec3d(x, y, z));
}

void
AnalyticalGeometry::rotate(const Quatd& r)
{
    m_orientation = r * m_orientation;
}

void
AnalyticalGeometry::rotate(const Mat3d& r)
{
    this->rotate(Quatd(r));
}

void
AnalyticalGeometry::rotate(const Vec3d& axis, const double& angle)
{
    this->rotate(Quatd(Eigen::AngleAxisd(angle, axis)));
}

const Vec3d&
AnalyticalGeometry::getPosition() const
{
    return m_position;
}

void
AnalyticalGeometry::setPosition(const Vec3d& position)
{
    m_position = position;
}

void
AnalyticalGeometry::setPosition(const double& x,
                                const double& y,
                                const double& z)
{
    this->setPosition(Vec3d(x, y, z));
}

const Quatd&
AnalyticalGeometry::getOrientation() const
{
    return m_orientation;
}

void
AnalyticalGeometry::setOrientation(const Quatd& orientation)
{
    m_orientation = orientation;
}

void
AnalyticalGeometry::setOrientation(const Mat3d& orientation)
{
    this->setOrientation(Quatd(orientation));
}

void
AnalyticalGeometry::setOrientation(const Vec3d& axis, const double& angle)
{
    this->setOrientation(Quatd(Eigen::AngleAxisd(angle, axis)));
}
}
