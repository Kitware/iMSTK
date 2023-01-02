/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMath.h"
#include "imstkTypes.h"

#include <array>

using namespace imstk;

///
/// \brief Inclusively checks if range (a,b) and (c,d) overlap in 1d, assuming a<b and c<d
///
static bool
isIntersect(const double a, const double b, const double c, const double d)
{
    return ((a <= d && a >= c) || (c <= b && c >= a)) ? true : false;
}

///
/// \brief Returns if intersected
/// exclusive on both sides
///
static bool
segmentToPlane(const Vec3d& a, const Vec3d& b, const Vec3d& n, const Vec3d& planePt, Vec3d& intersectionPt)
{
    Vec3d  dir    = b - a;
    double length = dir.norm();
    dir.normalize();
    double denom = n.dot(dir);

    // Line is tangent, or nearly
    if (std::abs(denom) < 0.00000001)
    {
        return false;
    }

    double t = (planePt - a).dot(n) / denom;

    // Starts outside of plane
    if (t < 0.0)
    {
        return false;
    }
    if (t > length)
    {
        return false;
    }

    intersectionPt = a + t * dir;
    return true;
}

static void
orientTet(std::array<Vec3d, 4>& tet)
{
    Vec3d a = tet[1] - tet[0];
    Vec3d b = tet[2] - tet[0];
    Vec3d c = tet[3] - tet[0];
    if (a.cross(b).dot(c) < 0)
    {
        std::swap(tet[2], tet[3]);
    }
}

///
/// \brief Split tet with plane. Returns if intersected with plane
///
static bool
splitTet(const std::array<Vec3d, 4>& inputTetVerts,
         const Vec3d& planePos, const Vec3d& planeNormal,
         std::list<std::array<Vec3d, 4>>& resultTetVerts)
{
    bool side[4];
    int  outCount = 0; // Num vertices that lie in front of plane
    int  inCount  = 0; // Num vertices that lie behind plane
    for (int j = 0; j < 4; j++)
    {
        const Vec3d& vert = inputTetVerts[j];
        const double proj = (vert - planePos).dot(planeNormal);
        if (proj >= 0)
        {
            outCount++;
            side[j] = false;
        }
        if (proj < 0)
        {
            side[j] = true;
            inCount++;
        }
    }

    // If all vertices lie on one side then its not intersecting
    if (outCount == 0 || inCount == 0)
    {
        return false;
    }

    Vec3d               iPts[4];
    std::pair<int, int> iEdges[4];
    int                 iPtSize = 0;
    // For every segment of the tetrahedron
    for (int j = 0; j < 4; j++)
    {
        for (int k = j + 1; k < 4; k++)
        {
            // If that segment has verts on both sides of the plane, it crosses
            if (side[j] != side[k])
            {
                // Compute intersection point
                Vec3d iPt;
                if (segmentToPlane(inputTetVerts[j], inputTetVerts[k], planeNormal, planePos, iPt))
                {
                    iEdges[iPtSize] = std::pair<int, int>(j, k);
                    iPts[iPtSize]   = iPt;
                    iPtSize++;
                }
            }
        }
    }

    // There are two cases
    // either 3 verts on one side, 1 on the other
    // or 2 verts on one side, 2 on the other
    if (iPtSize == 3) // 3 intersection points
    {
        // Identify the vertex of the isolated vert
        int isolatedVertId = -1;
        int otherVertIds[3];
        {
            int falseCount = 0;
            int trueCount  = 0;
            // All true but one
            for (int i = 0; i < 4; i++)
            {
                trueCount  += static_cast<int>(side[i]);
                falseCount += static_cast<int>(!side[i]);
            }

            // False is the ioslated vert
            if (trueCount == 3 && falseCount == 1)
            {
                int otherVertSize = 0;
                for (int i = 0; i < 4; i++)
                {
                    if (side[i])
                    {
                        otherVertIds[otherVertSize++] = i;
                    }
                    else
                    {
                        isolatedVertId = i;
                    }
                }
            }
            // True is the isolated vert
            else // falseCount == 3 && trueCount == 1
            {
                int otherVertSize = 0;
                for (int i = 0; i < 4; i++)
                {
                    if (side[i])
                    {
                        isolatedVertId = i;
                    }
                    else
                    {
                        otherVertIds[otherVertSize++] = i;
                    }
                }
            }
        }

        // Now that we've identified the points that lie on which sides we can generate our 4 tets
        // we do so disregarding orientation

        // On one side of the plane we have a singular tet
        // formed by the intersection verts on the plane
        std::array<Vec3d, 4> tet1;
        tet1[0] = iPts[0];
        tet1[1] = iPts[1];
        tet1[2] = iPts[2];
        tet1[3] = inputTetVerts[isolatedVertId];
        orientTet(tet1);
        resultTetVerts.push_back(tet1);

        // On the other side of the plane we have a 6vert 5face polyhedron
        // that is split into 3 tets here
        // It's like a triangular prism, but not a prism.
        // Two triangular faces, 3 quad faces
        // One of the triangular faces is incident with the plane (formed by the 3 iPts)
        std::array<Vec3d, 4> tet2;
        tet2[0] = iPts[0];
        tet2[1] = iPts[1];
        tet2[2] = iPts[2];
        tet2[3] = inputTetVerts[otherVertIds[0]];
        orientTet(tet2);
        resultTetVerts.push_back(tet2);

        std::array<Vec3d, 4> tet3;
        tet3[0] = iPts[1];
        tet3[1] = iPts[2];
        tet3[2] = inputTetVerts[otherVertIds[0]];
        tet3[3] = inputTetVerts[otherVertIds[2]];
        orientTet(tet3);
        resultTetVerts.push_back(tet3);

        std::array<Vec3d, 4> tet4;
        tet4[0] = iPts[1];
        tet4[1] = inputTetVerts[otherVertIds[0]];
        tet4[2] = inputTetVerts[otherVertIds[1]];
        tet4[3] = inputTetVerts[otherVertIds[2]];
        orientTet(tet4);
        resultTetVerts.push_back(tet4);
    }
    else // 4 intersection points
    {
        // Identify the 2 vertices on each side
        int inVertIds[2];
        int outVertIds[2];
        int inVertSize  = 0;
        int outVertSize = 0;
        for (int i = 0; i < 4; i++)
        {
            if (side[i])
            {
                outVertIds[outVertSize++] = i;
            }
            else
            {
                inVertIds[inVertSize++] = i;
            }
        }

        // Outer wedge
        {
            // We know we have two verts on one side. outVertIds[0] and [1]
            // We have 4 verts on the intersecting quad
            // 2 of these verts connect to outVertIds[0]
            // the other 2 to outVertIds[1]
            // forming two triangles (ends of the wedge)

            // We need to know which 2 verts correspond to outVertIds[0]
            int iPts0[2];
            int iPts0Size = 0;
            int iPts1[2];
            int iPts1Size = 0;
            for (int i = 0; i < iPtSize; i++) // For every intersection point
            {
                if (iEdges[i].first == outVertIds[0]
                    || iEdges[i].second == outVertIds[0])
                {
                    iPts0[iPts0Size++] = i;
                }
                else
                {
                    iPts1[iPts1Size++] = i;
                }
            }

            std::array<Vec3d, 4> tet1;
            tet1[0] = iPts[iPts0[0]];
            tet1[1] = iPts[iPts0[1]];
            tet1[2] = inputTetVerts[outVertIds[0]];
            tet1[3] = inputTetVerts[outVertIds[1]];
            orientTet(tet1);
            resultTetVerts.push_back(tet1);

            std::array<Vec3d, 4> tet2;
            tet2[0] = iPts[iPts0[0]];
            tet2[1] = iPts[iPts0[1]];
            tet2[2] = iPts[iPts1[0]];
            tet2[3] = inputTetVerts[outVertIds[1]];
            orientTet(tet2);
            resultTetVerts.push_back(tet2);

            std::array<Vec3d, 4> tet3;
            tet3[0] = iPts[iPts0[1]];
            tet3[1] = iPts[iPts1[0]];
            tet3[2] = iPts[iPts1[1]];
            tet3[3] = inputTetVerts[outVertIds[1]];
            orientTet(tet3);
            resultTetVerts.push_back(tet3);
        }
        // Inner wedge
        {
            // We need to know which 2 verts correspond to outVertIds[0]
            int iPts0[2];
            int iPts0Size = 0;
            int iPts1[2];
            int iPts1Size = 0;
            for (int i = 0; i < iPtSize; i++) // For every intersection point
            {
                if (iEdges[i].first == inVertIds[0]
                    || iEdges[i].second == inVertIds[0])
                {
                    iPts0[iPts0Size++] = i;
                }
                else
                {
                    iPts1[iPts1Size++] = i;
                }
            }

            std::array<Vec3d, 4> tet1;
            tet1[0] = iPts[iPts0[0]];
            tet1[1] = iPts[iPts0[1]];
            tet1[2] = inputTetVerts[inVertIds[0]];
            tet1[3] = inputTetVerts[inVertIds[1]];
            orientTet(tet1);
            resultTetVerts.push_back(tet1);

            std::array<Vec3d, 4> tet2;
            tet2[0] = iPts[iPts0[0]];
            tet2[1] = iPts[iPts0[1]];
            tet2[2] = iPts[iPts1[0]];
            tet2[3] = inputTetVerts[inVertIds[1]];
            orientTet(tet2);
            resultTetVerts.push_back(tet2);

            std::array<Vec3d, 4> tet3;
            tet3[0] = iPts[iPts0[1]];
            tet3[1] = iPts[iPts1[0]];
            tet3[2] = iPts[iPts1[1]];
            tet3[3] = inputTetVerts[inVertIds[1]];
            orientTet(tet3);
            resultTetVerts.push_back(tet3);
        }
    }
    return true;
}

///
/// \brief
/// \param Tetrahedron vertices
/// \param plane origin
/// \param plane basis x
/// \param plane length along x
/// \param plane basis y
/// \param plane width along y
/// \param plane normal, should be x.cross(y).normalized()
/// \return Whether to split or not
///
static std::list<std::array<Vec3d, 4>>
split(const std::array<Vec3d, 4>& inputTetVerts,
      const Vec3d& planeOrigin,
      const Vec3d& u, const double width,
      const Vec3d& v, const double height,
      const Vec3d& n)
{
    bool  side[4];
    Vec3d proj[4];
    int   outCount = 0; // Num vertices that lie in front of plane
    int   inCount  = 0; // Num vertices that lie behind plane
    for (int i = 0; i < 4; i++)
    {
        const Vec3d& vert = inputTetVerts[i];
        proj[i][2] = (vert - planeOrigin).dot(n);
        if (proj[i][2] >= 0)
        {
            outCount++;
            side[i] = false;
        }
        if (proj[i][2] < 0)
        {
            side[i] = true;
            inCount++;
        }
    }
    // If all vertices lie on one side then it's not intersecting
    if (outCount == 0 || inCount == 0)
    {
        return std::list<std::array<Vec3d, 4>>();
    }

    // Next cull by projection of bounds on plane (in a SAT manner)
    Vec2d min = Vec2d(IMSTK_DOUBLE_MAX, IMSTK_DOUBLE_MAX);
    Vec2d max = Vec2d(IMSTK_DOUBLE_MIN, IMSTK_DOUBLE_MIN);
    for (int i = 0; i < 4; i++)
    {
        // Project onto the basis of the plane
        const Vec3d& vert = inputTetVerts[i];
        proj[i][0] = (vert - planeOrigin).dot(u);
        proj[i][1] = (vert - planeOrigin).dot(v);

        min[0] = std::min(proj[i][0], min[0]);
        max[0] = std::max(proj[i][0], max[0]);

        min[1] = std::min(proj[i][1], min[1]);
        max[1] = std::max(proj[i][1], max[1]);
    }

    // If either range is not intersecting then the plane is not within the
    // bounds of the finite plane/quad
    if (!isIntersect(min[0], max[0], -width, width)
        || !isIntersect(min[1], max[1], -height, height))
    {
        return std::list<std::array<Vec3d, 4>>();
    }

    // Perform split with the plane and return resulting tets
    std::list<std::array<Vec3d, 4>> newTets;
    splitTet(inputTetVerts, planeOrigin, n, newTets);
    return newTets;
}

///
/// \brief Returns if plane intersects tet
///
static bool
splitTest(const std::array<Vec3d, 4>& inputTetVerts,
          const Vec3d& planeOrigin,
          const Vec3d& u, const double width,
          const Vec3d& v, const double height,
          const Vec3d& n)
{
    bool  side[4];
    Vec3d proj[4];
    int   outCount = 0; // Num vertices that lie in front of plane
    int   inCount  = 0; // Num vertices that lie behind plane
    for (int i = 0; i < 4; i++)
    {
        const Vec3d& vert = inputTetVerts[i];
        proj[i][2] = (vert - planeOrigin).dot(n);
        if (proj[i][2] >= 0)
        {
            outCount++;
            side[i] = false;
        }
        if (proj[i][2] < 0)
        {
            side[i] = true;
            inCount++;
        }
    }
    // If all vertices lie on one side then it's not intersecting
    if (outCount == 0 || inCount == 0)
    {
        return false;
    }

    // Next cull by projection of bounds on plane (in a SAT manner)
    Vec2d min = Vec2d(IMSTK_DOUBLE_MAX, IMSTK_DOUBLE_MAX);
    Vec2d max = Vec2d(IMSTK_DOUBLE_MIN, IMSTK_DOUBLE_MIN);
    for (int i = 0; i < 4; i++)
    {
        // Project onto the basis of the plane
        const Vec3d& vert = inputTetVerts[i];
        proj[i][0] = (vert - planeOrigin).dot(u);
        proj[i][1] = (vert - planeOrigin).dot(v);

        min[0] = std::min(proj[i][0], min[0]);
        max[0] = std::max(proj[i][0], max[0]);

        min[1] = std::min(proj[i][1], min[1]);
        max[1] = std::max(proj[i][1], max[1]);
    }

    // If either range is not intersecting then the plane is not within the
    // bounds of the finite plane/quad
    if (!isIntersect(min[0], max[0], -width, width)
        || !isIntersect(min[1], max[1], -height, height))
    {
        return false;
    }

    return true;
}