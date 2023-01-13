/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCollisionUtils.h"

namespace imstk
{
namespace CollisionUtils
{
bool
testLineToLineAabb(const double x1, const double y1, const double z1,
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

    return testAabbToAabb(min1_x - prox1, max1_x + prox1, min1_y - prox1, max1_y + prox1,
                          min1_z - prox1, max1_z + prox1, min2_x - prox2, max2_x + prox2,
                          min2_y - prox2, max2_y + prox2, min2_z - prox2, max2_z + prox2);
}

double
pointSegmentClosestDistance(const Vec3d& point, const Vec3d& x1, const Vec3d& x2)
{
    Vec3d  dx = x2 - x1;
    double m2 = dx.squaredNorm();
    if (m2 < 1e-20)
    {
        return (point - x1).norm();
    }

    // find parameter value of closest point on segment
    double s12 = dx.dot(x2 - point) / m2;

    if (s12 < 0)
    {
        s12 = 0;
    }
    else if (s12 > 1.0)
    {
        s12 = 1.0;
    }

    return (point - (s12 * x1 + (1.0 - s12) * x2)).eval().norm();
}

double
pointTriangleClosestDistance(const Vec3d& point, const Vec3d& x1, const Vec3d& x2, const Vec3d& x3)
{
    int         unusedType;
    const Vec3d closestPtOnTri = closestPointOnTriangle(point, x1, x2, x3, unusedType);

    return (point - closestPtOnTri).norm();
}

Vec3d
closestPointOnSegment(const Vec3d& point, const Vec3d& x1, const Vec3d& x2, int& caseType)
{
    Vec3d  dx = x2 - x1;
    double m2 = dx.squaredNorm();
    if (m2 < 1e-20)
    {
        caseType = 0;
        return x1;
    }

    // find parameter value of closest point on segment
    double s12 = dx.dot(x2 - point) / m2;

    if (s12 < 0)
    {
        s12      = 0;
        caseType = 1;
    }
    else if (s12 > 1.0)
    {
        s12      = 1.0;
        caseType = 0;
    }
    else
    {
        caseType = 2;
    }
    return (s12 * x1 + (1.0 - s12) * x2).eval();
}

Vec3d
closestPointOnTriangle(const Vec3d& p, const Vec3d& a, const Vec3d& b, const Vec3d& c, int& caseType)
{
    // Assumes counterclockwise indexed triangle ABC

    const Vec3d ab = b - a;
    const Vec3d ac = c - a;
    const Vec3d ap = p - a;

    const double d1 = ab.dot(ap);
    const double d2 = ac.dot(ap);
    caseType = -1;

    // Check if closest point on triangle is point A
    if (d1 <= 0.0 && d2 <= 0.0)
    {
        caseType = 0;
    }

    // Check if P in region outside B, so B is closest point
    const Vec3d  bp = p - b;
    const double d3 = ab.dot(bp);
    const double d4 = ac.dot(bp);
    if (d3 >= 0.0 && d4 <= d3)
    {
        caseType = 1;
    }

    // Check if P in vertex region outside C
    const Vec3d  cp = p - c;
    const double d5 = ab.dot(cp);
    const double d6 = ac.dot(cp);
    if (d6 >= 0.0 && d5 <= d6)
    {
        caseType = 2;
    }

    // Check if P in edge region of AB, if so return projection of P onto AB
    const double vc = d1 * d4 - d3 * d2;
    // if (vc <= 0.0 && d1 >= 0.0 && d3 <= 0.0)
    if (vc <= 0.0 && d1 > 0.0 && d3 < 0.0)
    {
        caseType = 3;
    }

    // Check if P in edge region of BC, if so return projection of P onto BC
    const double va = d3 * d6 - d5 * d4;
    if (va <= 0.0 && (d4 - d3) > 0.0 && (d5 - d6) > 0.0)
    {
        caseType = 4;
    }

    // Check if P in edge region of AC, if so return projection of P onto AC
    const double vb = d5 * d2 - d1 * d6;
    if (vb <= 0.0 && d2 > 0.0 && d6 < 0.0)
    {
        caseType = 5;
    }

    // If neartest point is not a point or edge, then it must be inside of the face
    if (caseType == -1)
    {
        caseType = 6;
    }

    // Variables for switch
    double v, w, denom;

    switch (caseType)
    {
    case 0: // Neartest point it A
        return a;
    case 1: // Neartest point it B
        return b;
    case 2: // Neartest point it C
        return c;
    case 3: // Neartest point on edge AB
        v = d1 / (d1 - d3);
        return a + v * ab;
    case 4:                     // Neartest point on edge BC
        w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        return b + w * (c - b); // barycentric coordinates (0,1-w,w)
    case 5:                     // Neartest point on edge AC
        w = d2 / (d2 - d6);
        return a + w * ac;      // barycentric coordinates (1-w,0,w)
    case 6:                     // Neartest point on face
        denom = 1.0 / (va + vb + vc);
        v     = vb * denom;
        w     = vc * denom;
        return a + ab * v + ac * w; // = u*a + v*b + w*c, u = va * denom = 1.0f-v-w
    }
}

int
triangleToTriangle(
    const Vec3i& tri_a, const Vec3i& tri_b,
    const Vec3d& p0_a, const Vec3d& p1_a, const Vec3d& p2_a,
    const Vec3d& p0_b, const Vec3d& p1_b, const Vec3d& p2_b,
    std::pair<Vec2i, Vec2i>& edgeContact,
    std::pair<int, Vec3i>& vertexTriangleContact,
    std::pair<Vec3i, int>& triangleVertexContact)
{
    // \todo: One edge case where both triangles are coplanar with vertices from the other
    int contactType = -1;

    const std::array<Vec3d, 3> triAVerts = { p0_a, p1_a, p2_a };
    const std::array<Vec3d, 3> triBVerts = { p0_b, p1_b, p2_b };

    // Edges of the first triangle
    const std::pair<Vec3d, Vec3d> triAEdges[3]{
        { triAVerts[0], triAVerts[1] },
        { triAVerts[0], triAVerts[2] },
        { triAVerts[1], triAVerts[2] }
    };

    // Edges of the second triangle
    const std::pair<Vec3d, Vec3d> triBEdges[3]{
        { triBVerts[0], triBVerts[1] },
        { triBVerts[0], triBVerts[2] },
        { triBVerts[1], triBVerts[2] }
    };

    // Test if segments of a intersected triangle b
    bool aIntersected[3];
    for (int i = 0; i < 3; i++)
    {
        aIntersected[i] = CollisionUtils::testSegmentTriangle(triAEdges[i].first, triAEdges[i].second,
            triBVerts[0], triBVerts[1], triBVerts[2]);
    }

    // Count number of edge-triangle intersections
    const int numIntersectionsA = aIntersected[0] + aIntersected[1] + aIntersected[2];
    if (numIntersectionsA == 2)
    {
        if (aIntersected[0])
        {
            const int vertIdx = aIntersected[1] ? tri_a[0] : tri_a[1];
            vertexTriangleContact = std::pair<int, Vec3i>(vertIdx, tri_b);
        }
        else
        {
            vertexTriangleContact = std::pair<int, Vec3i>(tri_a[2], tri_b);
        }
        contactType = 1;
    }
    else if (numIntersectionsA == 1)
    {
        Vec2i edgeIdA;
        if (aIntersected[0])
        {
            edgeIdA = { tri_a[0], tri_a[1] };
        }
        else if (aIntersected[1])
        {
            edgeIdA = { tri_a[0], tri_a[2] };
        }
        else
        {
            edgeIdA = { tri_a[1], tri_a[2] };
        }

        bool bFound = false; // Due to numerical round-off errors, the other triangle may not intersect with the current one

        // Find the only edge of triangle2 that intersects with triangle1s
        Vec2i edgeIdB;
        for (int i = 0; i < 3; i++)
        {
            if (CollisionUtils::testSegmentTriangle(triBEdges[i].first, triBEdges[i].second,
                triAVerts[0], triAVerts[1], triAVerts[2]))
            {
                if (i == 0)
                {
                    edgeIdB = { tri_b[0], tri_b[1] };
                }
                else if (i == 1)
                {
                    edgeIdB = { tri_b[0], tri_b[2] };
                }
                else
                {
                    edgeIdB = { tri_b[1], tri_b[2] };
                }
                bFound = true;
                break;
            }
        }
        // If there is numIntersections == 1 doesn't this garuntee one will be found?
        if (bFound)
        {
            edgeContact = std::pair<Vec2i, Vec2i>(edgeIdA, edgeIdB);
            contactType = 0;
        }
    }
    else
    {
        // Test if segments of b intersected triangle a
        bool bIntersected[3];
        for (int i = 0; i < 3; i++)
        {
            bIntersected[i] = CollisionUtils::testSegmentTriangle(triBEdges[i].first, triBEdges[i].second,
                triAVerts[0], triAVerts[1], triAVerts[2]);
        }

        // We don't need to cover edge-edge case since its symmetric and the other one should have caught it
        const int numIntersectionsB = bIntersected[0] + bIntersected[1] + bIntersected[2];
        if (numIntersectionsB == 2)
        {
            if (bIntersected[0])
            {
                const int vertIdx = bIntersected[1] ? tri_b[0] : tri_b[1];
                triangleVertexContact = std::pair<Vec3i, int>(tri_a, vertIdx);
            }
            else
            {
                triangleVertexContact = std::pair<Vec3i, int>(tri_a, tri_b[2]);
            }
            contactType = 2;
        }
    }
    return contactType;
}
} // namespace CollisionUtils
} // namespace imstk