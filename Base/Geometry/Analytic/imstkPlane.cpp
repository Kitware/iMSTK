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
    Geometry::print();
    LOG(INFO) << "Width: " << m_width;
}

double
Plane::getVolume() const
{
    return 0.0;
}

Vec3d
Plane::getNormal() const
{
    return m_normal;
}

void
Plane::setNormal(const Vec3d& normal)
{
    if(normal == Vec3d::Zero())
    {
        LOG(WARNING) << "Plane::setNormal: can't set normal to zero vector";
        return;
    }
    m_normal = normal;
}

const double&
Plane::getWidth() const
{
    return m_width;
}

void
Plane::setWidth(const double& width)
{
    if(width <= 0)
    {
        LOG(WARNING) << "Plane::setWidth error: width should be positive.";
        return;
    }
    m_width = width;
}

} // imstk
