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

#include "imstkSPHHemorrhage.h"
#include <iostream>


namespace imstk
{
SPHHemorrhage::SPHHemorrhage(const Vec3d& center, const double radius, const double area, const Vec3d& normal) :
    m_center(center), m_radius(radius), m_area(area)
{
    m_normal = normal.normalized();
}

bool SPHHemorrhage::pointCrossedHemorrhagePlane(const Vec3d& oldPosition, const Vec3d& newPosition)
{
    // todo - loop through points that are near hemorrhage plane instead of all points
    const double dist = m_normal.dot(newPosition - m_center);

    if (m_normal.dot(oldPosition - m_center) < 0 && dist > 0)
    {
        // particle has crossed plane in the correct direction
        // however, we still need to determine if the particle is in the domain of the hemorrhage area
        // project point onto plane
        const Vec3d pointOnPlane = newPosition - dist * m_normal;
        // check if point is farther than radius from center point
        const double distFromCenter = (pointOnPlane - m_center).norm();

        if (distFromCenter <= m_radius)
        {
            return true;
        }
    }
    return false;
}
} // end namespace imstk
