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

#include "imstkCollisionUtils.h"

namespace imstk
{
namespace CollisionUtils
{
bool
testLineToLineAABB(const double x1, const double y1, const double z1,
                   const double x2, const double y2, const double z2,
                   const double x3, const double y3, const double z3,
                   const double x4, const double y4, const double z4,
                   const double prox1 /*= VERY_SMALL_EPSILON_D*/, const double prox2 /*= VERY_SMALL_EPSILON_D*/)
{
    double min1_x, max1_x, min1_y, max1_y, min1_z, max1_z;

    if (x1 < x2)
    {
        min1_x = x1;
        max1_x = x2;
    }
    else
    {
        min1_x = x2;
        max1_x = x1;
    }

    if (y1 < y2)
    {
        min1_y = y1;
        max1_y = y2;
    }
    else
    {
        min1_y = y2;
        max1_y = y1;
    }

    if (z1 < z2)
    {
        min1_z = z1;
        max1_z = z2;
    }
    else
    {
        min1_z = z2;
        max1_z = z1;
    }

    double min2_x, max2_x, min2_y, max2_y, min2_z, max2_z;

    if (x3 < x4)
    {
        min2_x = x3;
        max2_x = x4;
    }
    else
    {
        min2_x = x4;
        max2_x = x3;
    }

    if (y3 < y4)
    {
        min2_y = y3;
        max2_y = y4;
    }
    else
    {
        min2_y = y4;
        max2_y = y3;
    }

    if (z3 < z4)
    {
        min2_z = z3;
        max2_z = z4;
    }
    else
    {
        min2_z = z4;
        max2_z = z3;
    }

    return testAABBToAABB(min1_x - prox1, max1_x + prox1, min1_y - prox1, max1_y + prox1,
                          min1_z - prox1, max1_z + prox1, min2_x - prox2, max2_x + prox2,
                          min2_y - prox2, max2_y + prox2, min2_z - prox2, max2_z + prox2);
}

bool
segmentIntersectsTriangle(const Vec3r& pA, const Vec3r& pB,
                          const Vec3r& v0, const Vec3r& v1, const Vec3r& v2)
{
    static const Real EPSILON = 1e-8;
    Vec3r             dirAB   = pB - pA;
    const Real        lAB     = dirAB.norm();
    if (lAB < 1e-8)
    {
        return false;
    }
    dirAB /= lAB;

    Vec3r edge1 = v1 - v0;
    Vec3r edge2 = v2 - v0;
    Vec3r h     = dirAB.cross(edge2);
    Real  a     = edge1.dot(h);
    if (a > -EPSILON && a < EPSILON)
    {
        return false;    // This ray is parallel to this triangle.
    }
    Real  f = Real(1.0) / a;
    Vec3r s = pA - v0;
    Real  u = f * s.dot(h);
    if (u < Real(0) || u > Real(1.0))
    {
        return false;
    }
    Vec3r q = s.cross(edge1);
    Real  v = f * dirAB.dot(q);
    if (v < Real(0) || u + v > Real(1.0))
    {
        return false;
    }
    // At this stage we can compute t to find out where the intersection point is on the line.
    Real t = f * edge2.dot(q);

    // ray - triangle intersection
    return (t > EPSILON && t + EPSILON < lAB);
}

Real
pointSegmentClosestDistance(const Vec3r& point, const Vec3r& x1, const Vec3r& x2)
{
    Vec3r dx = x2 - x1;
    Real  m2 = dx.squaredNorm();
    if (m2 < Real(1e-20))
    {
        return (point - x1).norm();
    }

    // find parameter value of closest point on segment
    Real s12 = dx.dot(x2 - point) / m2;

    if (s12 < 0)
    {
        s12 = 0;
    }
    else if (s12 > 1.0)
    {
        s12 = 1.0;
    }

    return (point - s12 * x1 + (1.0 - s12) * x2).norm();
}

Real
pointTriangleClosestDistance(const Vec3r& point, const Vec3r& x1, const Vec3r& x2, const Vec3r& x3)
{
    // first find barycentric coordinates of closest point on infinite plane
    Vec3r x13(x1 - x3), x23(x2 - x3), xp3(point - x3);
    Real  m13 = x13.squaredNorm(), m23 = x23.squaredNorm(), d = x13.dot(x23);

    Real invdet = 1.0 / std::max(m13 * m23 - d * d, 1e-30);
    Real a = x13.dot(xp3), b = x23.dot(xp3);

    // Barycentric coordinates
    Real w23 = invdet * (m23 * a - d * b);
    Real w31 = invdet * (m13 * b - d * a);
    Real w12 = 1.0 - w23 - w31;

    if (w23 >= 0 && w31 >= 0 && w12 >= 0)  // inside the triangle
    {
        return (point - w23 * x1 + w31 * x2 + w12 * x3).norm();
    }
    else
    {
        if (w23 > 0)       //this rules out edge 2-3
        {
            return std::min(pointSegmentClosestDistance(point, x1, x2),
                            pointSegmentClosestDistance(point, x1, x3));
        }
        else if (w31 > 0)  //this rules out edge 1-3
        {
            return std::min(pointSegmentClosestDistance(point, x1, x2),
                            pointSegmentClosestDistance(point, x2, x3));
        }
        else               //w12 must be >0, ruling out edge 1-2
        {
            return std::min(pointSegmentClosestDistance(point, x1, x3),
                            pointSegmentClosestDistance(point, x2, x3));
        }
    }
}
} // namespace CollisionUtils
} // imstk
