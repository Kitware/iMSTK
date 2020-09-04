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

#include "imstkCompositeImplicitGeometry.h"

namespace imstk
{
double
CompositeImplicitGeometry::getFunctionValue(const Vec3d& pos) const
{
    // Initialize everything to outside
    double distA = std::numeric_limits<double>::max();
    for (auto i = geometries.begin(); i != geometries.end(); i++)
    {
        const GeometryBoolPair& pair  = *i;
        double                  distB = pair.first->getFunctionValue(pos);
        switch (pair.second)
        {
        case GeometryBoolType::DIFFERENCE:
            distA = std::max(distA, -distB);
            break;
        case GeometryBoolType::INTERSECTION:
            distA = std::max(distA, distB);
            break;
        case GeometryBoolType::UNION:
            distA = std::min(distA, distB);
            break;
        }
    }
    return distA;
}
}