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

#pragma once

#include "imstkMath.h"
#include <algorithm>

namespace imstk
{
namespace CollisionUtils
{
///
/// \brief Do ranges [a,b] and [c,d] intersect?
///
inline bool
isIntersect(const double a, const double b, const double c, const double d)
{
    return ((a <= d && a >= c) || (c <= b && c >= a)) ? true : false;
}

///
/// \brief Check if two AABBs are intersecting
///
inline bool
testAABBToAABB(const double min1_x, const double max1_x,
               const double min1_y, const double max1_y,
               const double min1_z, const double max1_z,
               const double min2_x, const double max2_x,
               const double min2_y, const double max2_y,
               const double min2_z, const double max2_z)
{
    return (isIntersect(min1_x, max1_x, min2_x, max2_x)
            && isIntersect(min1_y, max1_y, min2_y, max2_y)
            && isIntersect(min1_z, max1_z, min2_z, max2_z));
}

///
/// \brief Check if triangle and point are intersecting with AABB test
///
inline bool
testPointToTriAABB(const double x1, const double y1, const double z1,
                   const double x2, const double y2, const double z2,
                   const double x3, const double y3, const double z3,
                   const double x4, const double y4, const double z4,
                   const double prox1, const double prox2)
{
    const auto min_x = std::min({ x2, x3, x4 });
    const auto max_x = std::max({ x2, x3, x4 });
    const auto min_y = std::min({ y2, y3, y4 });
    const auto max_y = std::max({ y2, y3, y4 });
    const auto min_z = std::min({ z2, z3, z4 });
    const auto max_z = std::max({ z2, z3, z4 });

    return testAABBToAABB(x1 - prox1, x1 + prox1, y1 - prox1, y1 + prox1,
                          z1 - prox1, z1 + prox1, min_x - prox2, max_x + prox2,
                          min_y - prox2, max_y + prox2, min_z - prox2, max_z + prox2);
}

///
/// \brief Check if two lines are intersecting with AABB intersection test
/// \param x1/y1/z1 Coordinates of the first end-point of the first line
/// \param x2/y2/z2 Coordinates of the second end-point of the first line
/// \param x3/y3/z3 Coordinates of the first end-point of the second line
/// \param x4/y4/z4 Coordinates of the second end-point of the second line
/// \param prox1 Round-off precision for the test
/// \param prox2 Round-off precision for the test
///
bool testLineToLineAABB(const double x1, const double y1, const double z1,
                        const double x2, const double y2, const double z2,
                        const double x3, const double y3, const double z3,
                        const double x4, const double y4, const double z4,
                        const double prox1 = VERY_SMALL_EPSILON_D, const double prox2 = VERY_SMALL_EPSILON_D);

///
/// \brief Check if two lines are intersecting with AABB intersection test
/// \param p1A The first end-point of the first line
/// \param p1B The second end-point of the first line
/// \param p2A The first end-point of the second line
/// \param p2B The second end-point of the second line
/// \param prox1 Round-off precision for the test
/// \param prox2 Round-off precision for the test
///
inline bool
testLineToLineAABB(const Vec3r& p1A, const Vec3r& p1B,
                   const Vec3r& p2A, const Vec3r& p2B,
                   const double prox1 = VERY_SMALL_EPSILON_D, const double prox2 = VERY_SMALL_EPSILON_D)
{
    const Real* p1Aptr = &p1A[0];
    const Real* p1Bptr = &p1B[0];
    const Real* p2Aptr = &p2A[0];
    const Real* p2Bptr = &p2B[0];
    return testLineToLineAABB(p1Aptr[0], p1Aptr[1], p1Aptr[2],
            p1Bptr[0], p1Bptr[1], p1Bptr[2],
            p2Aptr[0], p2Aptr[1], p2Aptr[2],
            p2Bptr[0], p2Bptr[1], p2Bptr[2],
            prox1, prox2);
}

///
/// \brief Test if the segment define by points pA-pB intersects with triangle using Moller–Trumbore algorithm
///
bool segmentIntersectsTriangle(const Vec3r& pA, const Vec3r& pB,
                               const Vec3r& v0, const Vec3r& v1, const Vec3r& v2);

///
/// \brief Compute closest distance from a point to a segment x1-x2
///
Real pointSegmentClosestDistance(const Vec3r& point, const Vec3r& x1, const Vec3r& x2);

///
/// \brief Compute closest distance from a point to a triangle x1-x2-x3
///
Real pointTriangleClosestDistance(const Vec3r& point, const Vec3r& x1, const Vec3r& x2, const Vec3r& x3);
} // namespace CollisionUtils
} // imstk
