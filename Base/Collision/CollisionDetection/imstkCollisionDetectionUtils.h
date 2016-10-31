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

#ifndef imstkCollisionDetectionUtils_h
#define imstkCollisionDetectionUtils_h

#include <algorithm>

namespace imstk
{

///
/// \brief
///
inline bool
isIntersect(const double& a, const double& b, const double& c, const double& d)
{
    return ((a <= d && a >= c) || (c <= b && c >= a)) ? true : false;
}

///
/// \brief Check if two AABBs are intersecting
///
inline bool
testAabbToAabb(const double& min1_x, const double& max1_x,
               const double& min1_y, const double& max1_y,
               const double& min1_z, const double& max1_z,
               const double& min2_x, const double& max2_x,
               const double& min2_y, const double& max2_y,
               const double& min2_z, const double& max2_z)
{
    return (isIntersect(min1_x, max1_x, min2_x, max2_x) &&
        isIntersect(min1_y, max1_y, min2_y, max2_y) &&
        isIntersect(min1_z, max1_z, min2_z, max2_z));
}

///
/// \brief Check if two lines are intersecting with AABB intersection test
///
inline bool
testLineToLineAabb(const double& x1, const double& y1, const double& z1,
               const double& x2, const double& y2, const double& z2,
               const double& x3, const double& y3, const double& z3,
               const double& x4, const double& y4, const double& z4,
               const double& prox1, const double& prox2)
{
    double min1_x, max1_x, min1_y, max1_y, min1_z, max1_z;

    if (x1 < x2)
    {
        min1_x = x1; max1_x = x2;
    }
    else
    {
        min1_x = x2; max1_x = x1;
    }

    if (y1 < y2)
    {
        min1_y = y1; max1_y = y2;
    }
    else
    {
        min1_y = y2; max1_y = y1;
    }

    if (z1 < z2)
    {
        min1_z = z1; max1_z = z2;
    }
    else
    {
        min1_z = z2; max1_z = z1;
    }

    double min2_x, max2_x, min2_y, max2_y, min2_z, max2_z;

    if (x3 < x4)
    {
        min2_x = x3; max2_x = x4;
    }
    else
    {
        min2_x = x4; max2_x = x3;
    }

    if (y3 < y4)
    {
        min2_y = y3; max2_y = y4;
    }
    else
    {
        min2_y = y4; max2_y = y3;
    }

    if (z3 < z4)
    {
        min2_z = z3; max2_z = z4;
    }
    else
    {
        min2_z = z4; max2_z = z3;
    }

    return testAabbToAabb(min1_x - prox1, max1_x + prox1, min1_y - prox1, max1_y + prox1,
        min1_z - prox1, max1_z + prox1, min2_x - prox2, max2_x + prox2,
        min2_y - prox2, max2_y + prox2, min2_z - prox2, max2_z + prox2);
}

///
/// \brief Check if triangle and point are intersecting with AABB test
///
inline bool
testPointToTriAabb(const double& x1, const double& y1, const double& z1,
                   const double& x2, const double& y2, const double& z2,
                   const double& x3, const double& y3, const double& z3,
                   const double& x4, const double& y4, const double& z4,
                   const double& prox1, const double& prox2)
{
    double min_x, max_x, min_y, max_y, min_z, max_z;
    min_x = std::min(x2, std::min(x3, x4));
    max_x = std::max(x2, std::max(x3, x4));
    min_y = std::min(y2, std::min(y3, y4));
    max_y = std::max(y2, std::max(y3, y4));
    min_z = std::min(z2, std::min(z3, z4));
    max_z = std::max(z2, std::max(z3, z4));

    return testAabbToAabb(x1 - prox1, x1 + prox1, y1 - prox1, y1 + prox1,
        z1 - prox1, z1 + prox1, min_x - prox2, max_x + prox2,
        min_y - prox2, max_y + prox2, min_z - prox2, max_z + prox2);
}

}

#endif // ifndef imstkCollisionDetectionUtils_h
