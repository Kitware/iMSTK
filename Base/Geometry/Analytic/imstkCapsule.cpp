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

#include "imstkCapsule.h"

#include "g3log/g3log.hpp"

namespace imstk
{

void
Capsule::print() const
{
    Geometry::print();
    LOG(INFO) << "Radius: " << m_radius;
}

double
Capsule::getVolume() const
{
    return PI * m_radius * m_radius *(m_height + 4.0 / 3.0 * m_radius);
}

void
Capsule::setRadius(const double& r)
{
    if(r <= 0)
    {
        LOG(WARNING) << "Capsule::setRadius error: radius should be positive.";
        return;
    }
    m_radius = r;
}

void
Capsule::setHeight(const double& h)
{
    if (h <= 0)
    {
        LOG(WARNING) << "Capsule::setHeight error: height should be positive.";
        return;
    }
    m_height = h;
}

} // imstk
