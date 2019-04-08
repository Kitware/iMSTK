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

#include "imstkLight.h"

#include <g3log/g3log.hpp>

namespace imstk
{
void
Light::setFocalPoint(const float& x, const float& y, const float& z)
{
    if (x != 0 || y != 0 || z != 0)
    {
        m_focalPoint = Vec3f((float)x, (float)y, (float)z);
    }
    else
    {
        LOG(WARNING) << "Directional lights can't have focal point at (0, 0, 0)";
    }
}

//---------------------------------------------------------------------------------
} // imstk
