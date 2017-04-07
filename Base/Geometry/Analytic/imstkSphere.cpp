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

#include "g3log/g3log.hpp"

namespace imstk
{

void
Sphere::print() const
{
    Geometry::print();
    LOG(INFO) << "Radius: " << m_radius;
}

double
Sphere::getVolume() const
{
    return 4.0 / 3.0 * PI * m_radius * m_radius * m_radius;
}

const double&
Sphere::getRadius() const
{
    return m_radius;
}

void
Sphere::setRadius(const double& radius)
{
    if(radius <= 0)
    {
        LOG(WARNING) << "Sphere::setRadius error: radius should be positive.";
        return;
    }
    m_radius = radius;
}

} // imstk
