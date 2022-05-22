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
#include "imstkTypes.h"

#include <algorithm>
#include <array>

namespace imstk
{
namespace CollisionUtils
{
///
/// \brief Do ranges [\p a,\p b] and [\p c,\p d] intersect?
/// inclusive
/// a must be < b
/// c must be < d
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
testAabbToAabb(const double min1_x, const double max1_x,
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
testPointToTriAabb(const double x1, const double y1, const double z1,
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

    return testAabbToAabb(x1 - prox1, x1 + prox1, y1 - prox1, y1 + prox1,
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
bool testLineToLineAabb(const double x1, const double y1, const double z1,
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
testLineToLineAabb(const Vec3d& p1A, const Vec3d& p1B,
                   const Vec3d& p2A, const Vec3d& p2B,
                   const double prox1 = VERY_SMALL_EPSILON_D, const double prox2 = VERY_SMALL_EPSILON_D)
{
    const double* p1Aptr = &p1A[0];
    const double* p1Bptr = &p1B[0];
    const double* p2Aptr = &p2A[0];
    const double* p2Bptr = &p2B[0];
    return testLineToLineAabb(p1Aptr[0], p1Aptr[1], p1Aptr[2],
        p1Bptr[0], p1Bptr[1], p1Bptr[2],
        p2Aptr[0], p2Aptr[1], p2Aptr[2],
        p2Bptr[0], p2Bptr[1], p2Bptr[2],
        prox1, prox2);
}

///
/// \brief Check if an OBB and point intersect, no contact info
/// \param center of box
/// \param rotation matrix of box
/// \param x, y, z scalar extents
/// \param point to test
/// \return true if intersecting, false if not
///
inline bool
testOBBToPoint(
    const Vec3d& cubePos, const Mat3d& rot, const Vec3d extents,
    const Vec3d& pt)
{
    // We assume rot is an orthonormal rotation matrix (no shear or scale/normalized)
    // We then take the diff from the center of the cube and project onto each axes
    const Vec3d diff = (pt - cubePos);
    const Vec3d proj = rot.transpose() * diff; // dot product on each axes

    return
        (std::abs(proj[0]) < extents[0])
        && (std::abs(proj[1]) < extents[1])
        && (std::abs(proj[2]) < extents[2]);
}

///
/// \brief Check if an oriented cube and point intersect, with contact info (normal and surface contact point)
/// \return true if intersecting, false if not
///
inline bool
testOBBToPoint(
    const Vec3d& cubePos, const Mat3d& rot, const Vec3d extents,
    const Vec3d& pt,
    Vec3d& ptContactNormal, Vec3d& cubeContactPt, double& penetrationDepth)
{
    const Vec3d diff = (pt - cubePos);
    const Vec3d proj = rot.transpose() * diff;   // dot product on each axes

    bool inside[3] =
    {
        (std::abs(proj[0]) < extents[0]),
        (std::abs(proj[1]) < extents[1]),
        (std::abs(proj[2]) < extents[2])
    };
    bool isInsideCube = inside[0] && inside[1] && inside[2];

    if (isInsideCube)
    {
        // If inside, find closest face, use that distance
        penetrationDepth = std::numeric_limits<double>::max();
        for (int i = 0; i < 3; i++)
        {
            double       dist = proj[i];
            const Vec3d& axes = rot.col(i);

            if (dist < extents[i] && dist > 0.0)
            {
                const double unsignedDistToSide = (extents[i] - dist);
                if (unsignedDistToSide < penetrationDepth)
                {
                    cubeContactPt    = pt + unsignedDistToSide * axes;
                    penetrationDepth = unsignedDistToSide;
                    ptContactNormal  = axes;
                }
            }
            else if (dist > -extents[i] && dist < 0.0)
            {
                const double unsignedDistToSide = (extents[i] + dist);
                if (unsignedDistToSide < penetrationDepth)
                {
                    cubeContactPt    = pt - unsignedDistToSide * axes;
                    penetrationDepth = unsignedDistToSide;
                    ptContactNormal  = -axes;
                }
            }
        }
    }
    else
    {
        // If outside we need to also consider diagonal distance to corners and edges
        // Compute nearest point
        cubeContactPt   = cubePos;
        ptContactNormal = Vec3d::Zero();
        for (int i = 0; i < 3; i++)
        {
            double       dist = proj[i];
            const Vec3d& axes = rot.col(i);

            // If distance farther than the box extents, clamp to the box
            if (dist >= extents[i])
            {
                cubeContactPt   += extents[i] * axes;
                ptContactNormal += axes;
            }
            else if (dist <= -extents[i])
            {
                cubeContactPt   -= extents[i] * axes;
                ptContactNormal -= axes;
            }
        }
        ptContactNormal.normalize();
    }

    return isInsideCube;
}

///
/// \brief Check if plane and sphere intersect, no contact info
/// \return true if intersecting, false if not
///
inline bool
testPlaneToSphere(
    const Vec3d& planePt, const Vec3d& planeNormal,
    const Vec3d& spherePos, const double r)
{
    return (spherePos - planePt).dot(planeNormal) < r;
}

///
/// \brief Check if plane and sphere intersect, with contact info (normal and surface contact point)
/// \return true if intersecting, false if not
///
inline bool
testPlaneToSphere(
    const Vec3d& planePt, const Vec3d& planeNormal,
    const Vec3d& spherePos, const double r,
    Vec3d& planeContactPt, Vec3d& planeContactNormal,
    Vec3d& sphereContactPt, Vec3d& sphereContactNormal, double& penetrationDepth)
{
    const double d = (spherePos - planePt).dot(planeNormal);

    planeContactNormal  = -planeNormal;
    sphereContactNormal = planeNormal;
    planeContactPt      = spherePos - d * planeNormal;
    sphereContactPt     = spherePos - r * planeNormal;

    penetrationDepth = r - d;
    return d < r;
}

///
/// \brief Check if bidirectional plane and sphere intersect, with contact info
/// \return true if intersecting, false if not
///
inline bool
testBidirectionalPlaneToSphere(
    const Vec3d& planePt, const Vec3d& planeNormal,
    const Vec3d& spherePos, const double r,
    Vec3d& planeContactPt, Vec3d& planeContactNormal,
    Vec3d& sphereContactPt, Vec3d& sphereContactNormal, double& pentrationDepth)
{
    const double d = (spherePos - planePt).dot(planeNormal);

    planeContactPt = spherePos - d * planeNormal;

    if (d < 0.0)
    {
        planeContactNormal  = planeNormal;
        sphereContactNormal = -planeNormal;
        sphereContactPt     = spherePos + r * planeNormal;
    }
    else
    {
        planeContactNormal  = -planeNormal;
        sphereContactNormal = planeNormal;
        sphereContactPt     = spherePos - r * planeNormal;
    }

    pentrationDepth = r - std::abs(d);
    return pentrationDepth > 0.0;
}

///
/// \brief Check if sphere and sphere intersect, no contact info
/// \return true if intersecting, false if not
///
inline bool
testSphereToSphere(
    const Vec3d& sphereAPos, const double rA,
    const Vec3d& sphereBPos, const double rB)
{
    const double rSum = rA + rB;
    return (sphereBPos - sphereAPos).squaredNorm() < (rSum * rSum);
}

///
/// \brief Check if sphere and sphere intersect, with contact info (normal and surface contact point)
/// \return true if intersecting, false if not
///
inline bool
testSphereToSphere(
    const Vec3d& sphereAPos, const double rA,
    const Vec3d& sphereBPos, const double rB,
    Vec3d& sphereAContactPt, Vec3d& sphereAContactNormal,
    Vec3d& sphereBContactPt, Vec3d& sphereBContactNormal,
    double& depth)
{
    Vec3d dirAtoB = sphereBPos - sphereAPos;

    const double d = dirAtoB.norm();
    dirAtoB = dirAtoB / d;

    sphereAContactPt     = sphereAPos + dirAtoB * rA;
    sphereAContactNormal = -dirAtoB;
    sphereBContactPt     = sphereBPos - dirAtoB * rB;
    sphereBContactNormal = dirAtoB;
    depth = (rA + rB) - d;

    return depth > 0.0;
}

///
/// \brief Check if sphere and cylinder intersect, with contact info (normal and surface contact point)
/// \return true if intersecting, false if not
///
inline bool
testSphereToCylinder(
    const Vec3d& spherePos, const double rSphere,
    const Vec3d& cylinderPos, const Vec3d& cylinderAxis, const double rCylinder, const double cylinderLength,
    Vec3d& sphereContactPt, Vec3d& sphereContactNormal,
    Vec3d& cylinderContactPt, Vec3d& cylinderContactNormal,
    double& depth)
{
    const double cylHalfLength = cylinderLength * 0.5;

    // Compute distance
    const Vec3d cylToSphere = (spherePos - cylinderPos);
    const Vec3d n = cylinderAxis;

    // Get normal distance (along axes)
    const double distN    = n.dot(cylToSphere);
    const Vec3d  distNVec = distN * n;

    // Get perp distance (orthogonal to axes)
    const Vec3d  distPerpVec = cylToSphere - distNVec;    // Remove N component to get perp
    const double distPerp    = distPerpVec.norm();
    const Vec3d  perp = distPerpVec.normalized();

    // If the center of the sphere is within the length of the cylinder
    if (std::abs(distN) < cylHalfLength)
    {
        //  ______
        // |      |_
        // |     (__)
        // |      |
        //

        sphereContactPt   = spherePos - perp * rSphere;
        cylinderContactPt = cylinderPos + distNVec + perp * rCylinder;

        sphereContactNormal   = perp;
        cylinderContactNormal = -perp;
        depth = (rSphere + rCylinder) - distPerp;

        return depth < 0.0;
    }
    else
    {
        // \todo Does not return closest points, returns no points when no collision
        // slight math change needed (also add SDF function to cylinder)
        //    __           __
        //  _(__)_    ____(__)
        // |      |  |     |
        // |      |  |     |
        //

        // If any portion of the sphere is within the length of the cylinder
        if (std::abs(distN) < (cylHalfLength + rSphere))
        {
            // If the center of the sphere is within the circle of the caps
            //    __
            //  _(__)_
            // |      |
            // |      |
            //
            if (distPerp < rCylinder)
            {
                sphereContactPt   = spherePos - n * rSphere;
                cylinderContactPt = cylinderPos + n * cylHalfLength + distPerpVec;

                sphereContactNormal   = n;
                cylinderContactNormal = -n;
                depth = (rSphere + cylHalfLength) - std::abs(distN);

                return true;
            }
            // Finally the sphere vs circular cap edge/rim
            //       __
            //  ____(__)
            // |     |
            // |     |
            //
            else if (distPerp < (rCylinder + rSphere))
            {
                cylinderContactPt = cylinderPos + n * cylHalfLength + perp * rCylinder;
                const Vec3d  diagDiff = spherePos - cylinderContactPt;
                const double diagDist = diagDiff.norm();

                sphereContactNormal   = diagDiff / diagDist;
                cylinderContactNormal = -sphereContactNormal;

                sphereContactPt = spherePos + cylinderContactNormal * rSphere;
                depth = rSphere - diagDist;
                return true;
            }
        }
    }
    return false;
}

///
/// \brief Check if point and capsule intersect, no contact info
/// \return true if intersecting, false if not
///
inline bool
testCapsuleToPoint(
    const Vec3d& capsulePos, const Vec3d& capsuleAxis, const double capsuleLength, const double rCapsule,
    const Vec3d& point)
{
    // Two lines points
    const Vec3d a = capsulePos + 0.5 * capsuleAxis * capsuleLength;
    const Vec3d b = 2.0 * capsulePos - a;

    const Vec3d  pa = point - a;
    const Vec3d  ba = b - a;
    const double h  = std::min(std::max(pa.dot(ba) / ba.dot(ba), 0.0), 1.0);
    const double signedDist = ((pa - ba * h).norm() - rCapsule);
    return signedDist < 0.0;
}

///
/// \brief Check if point and capsule intersect, with contact info (normal and surface contact point)
/// \return true if intersecting, false if not
///
inline bool
testCapsuleToPoint(
    const Vec3d& capsulePos, const Vec3d& capsuleAxis, const double capsuleLength, const double rCapsule,
    const Vec3d& point,
    Vec3d& capsuleContactPt, Vec3d& capsuleContactNormal, Vec3d& pointContactNormal, double& depth)
{
    // Get position of end points of the capsule
    // \todo Fix this issue of extra computation in future
    const Vec3d  mid   = capsulePos;
    const Vec3d  p1    = mid + 0.5 * capsuleAxis * capsuleLength;
    const Vec3d  p0    = 2.0 * mid - p1;
    const Vec3d  pDiff = p1 - p0;
    const double pDiffSqrLength = pDiff.dot(pDiff);
    const double pDotp0 = pDiff.dot(p0);

    // First, check collision with bounding sphere
    if ((mid - point).norm() > (rCapsule + capsuleLength * 0.5))
    {
        return false;
    }

    // Do the actual check
    const double alpha = (point.dot(pDiff) - pDotp0) / pDiffSqrLength;
    Vec3d        closestPoint;
    if (alpha > 1.0)
    {
        closestPoint = p1;
    }
    else if (alpha < 0.0)
    {
        closestPoint = p0;
    }
    else
    {
        closestPoint = p0 + alpha * pDiff;
    }

    // If the point is inside the bounding sphere then the closest point
    // should be inside the capsule
    const Vec3d  diff = (point - closestPoint);
    const double dist = diff.norm();
    if (dist < rCapsule)
    {
        depth = rCapsule - dist;
        pointContactNormal   = diff.normalized();
        capsuleContactNormal = -pointContactNormal;
        capsuleContactPt     = closestPoint + pointContactNormal * rCapsule;
        return true;
    }
    return false;
}

///
/// \brief Check if point and cylinder intersect, with contact info (normal and surface contact point)
/// \return true if intersecting, false if not
///
inline bool
testCylinderToPoint(
    const Vec3d& cylinderPos, const Vec3d& cylinderAxis, const double cylinderLength, const double cylinderRadius,
    const Vec3d& point,
    Vec3d& cylinderContactPt, Vec3d& cylinderContactNormal, Vec3d& pointContactNormal, double& depth)
{
    // First, check collision with bounding sphere
    if ((cylinderPos - point).squaredNorm() > (cylinderRadius * cylinderRadius + 0.25 * cylinderLength * cylinderLength))
    {
        return false;
    }

    // Get position of end points of the cylinder
    const Vec3d  mid    = cylinderPos;
    const Vec3d  p1     = mid + 0.5 * cylinderAxis * cylinderLength;
    const Vec3d  p0     = 2.0 * mid - p1;
    const Vec3d  pDiff  = p1 - p0;
    const double pDotp0 = pDiff.dot(p0);

    // Do the actual check
    const double alpha = (point.dot(pDiff) - pDotp0) / (cylinderLength * cylinderLength);
    if ((alpha > 1.0) || (alpha < 0.0))
    {
        return false;
    }
    else
    {
        Vec3d        closestPointOnAxis = p0 + alpha * pDiff;
        const Vec3d  diff = (point - closestPointOnAxis);
        const double dist = diff.norm();
        if (dist < cylinderRadius)
        {
            const double distToEnd = (alpha * pDiff).norm();

            if (distToEnd < (cylinderRadius - dist))
            {
                depth = distToEnd;
                cylinderContactNormal = pDiff.normalized();
                pointContactNormal    = -cylinderContactNormal;
                cylinderContactPt     = point + pointContactNormal * distToEnd;
            }
            else if ((cylinderLength - distToEnd) < (cylinderRadius - dist))
            {
                depth = cylinderLength - distToEnd;
                cylinderContactNormal = -pDiff.normalized();
                pointContactNormal    = -cylinderContactNormal;
                cylinderContactPt     = point + pointContactNormal * (cylinderLength - distToEnd);
            }
            else
            {
                depth = cylinderRadius - dist;
                pointContactNormal    = diff.normalized();
                cylinderContactNormal = -pointContactNormal;
                cylinderContactPt     = closestPointOnAxis + pointContactNormal * cylinderRadius;
            }
            return true;
        }
        return false;
    }
}

///
/// \brief Check if sphere and point intersect, no contact info
/// \return true if intersecting, false if not
///
inline bool
testSphereToPoint(
    const Vec3d& spherePos, const double rSqr, const Vec3d& point)
{
    return (spherePos - point).squaredNorm() < rSqr;
}

///
/// \brief Check if sphere and point intersect, with contact info
/// \return true if intersecting, false if not
///
inline bool
testSphereToPoint(
    const Vec3d& spherePos, const double r, const Vec3d& point,
    Vec3d& sphereContactPt, Vec3d& sphereContactNormal, double& penetrationDepth)
{
    const Vec3d diff = point - spherePos;
    penetrationDepth    = diff.norm();
    sphereContactNormal = diff / penetrationDepth;
    sphereContactPt     = spherePos + sphereContactNormal * r;
    const double signedDist = penetrationDepth - r;
    penetrationDepth = std::abs(signedDist);
    return signedDist < 0.0;
}

///
/// \brief Check if plane and point intersect, no contact info
/// \return true if intersecting, false if not
///
inline bool
testPlaneToPoint(
    const Vec3d& planePt, const Vec3d& planeNormal,
    const Vec3d& point)
{
    return (point - planePt).dot(planeNormal) < 0.0;
}

///
/// \brief Check if plane and point intersect, with contact info
/// \return true if intersecting, false if not
///
inline bool
testPlaneToPoint(
    const Vec3d& planePt, const Vec3d& planeNormal,
    const Vec3d& point,
    Vec3d& planeContactPt, Vec3d& contactNormal, double& pointPenetrationDepth)
{
    contactNormal = planeNormal;
    const double d = (point - planePt).dot(planeNormal);
    planeContactPt = point - pointPenetrationDepth * contactNormal;
    pointPenetrationDepth = std::abs(d);
    return d < 0.0;
}

///
/// \brief Compute intersection point of plane and infinite line
///
inline bool
testPlaneLine(const Vec3d& p, const Vec3d& q,
              const Vec3d& planePt, const Vec3d& planeNormal, Vec3d& iPt)
{
    const Vec3d  n     = q - p;
    const double denom = n.dot(planeNormal);
    // Plane and line are parallel
    if (std::abs(denom) < IMSTK_DOUBLE_EPS)
    {
        return false;
    }
    // const Vec3d  dir = n.normalized();
    const double t = (planePt - p).dot(planeNormal) / denom;
    iPt = p + t * n;
    return true;
}

///
/// \brief Compute intersection of triangle vs segment and returns triangle interpolation
/// weights
///
static bool
testSegmentTriangle(
    const Vec3d& p, const Vec3d& q,
    const Vec3d& a, const Vec3d& b, const Vec3d& c,
    Vec3d& uvw)
{
    const Vec3d  n = q - p;
    const Vec3d  planeNormal = (b - a).cross(c - a);
    const double denom       = n.dot(planeNormal);
    // Plane and line are parallel
    if (std::abs(denom) < IMSTK_DOUBLE_EPS)
    {
        return false;
    }

    const double t1 = (a - p).dot(planeNormal);
    const double t2 = (a - q).dot(planeNormal);

    // Check if p and q lie on opposites side of the plane
    if ((t1 < 0.0 && t2 >= 0.0) || (t1 >= 0.0 && t2 < 0.0))
    {
        // \todo: Does planeNormal need to be normalized to get valid p + t1 * n, given division by denom
        //t1 /= denom;
        //t2 /= denom;
        uvw = baryCentric(p + t1 / denom * n, a, b, c);
        // Lastly check if the point on the plane p+t1*n is inside the triangle
        return (uvw[0] >= 0.0 && uvw[1] >= 0.0 && uvw[2] >= 0.0);
    }
    else
    {
        return false;
    }
}

///
/// \brief Test if the segment define by points pA-pB intersects with triangle using Moller–Trumbore algorithm
///
static bool
testSegmentTriangle(
    const Vec3d& p, const Vec3d& q,
    const Vec3d& a, const Vec3d& b, const Vec3d& c)
{
    const Vec3d  n = q - p;
    const Vec3d  planeNormal = (b - a).cross(c - a);
    const double denom       = n.dot(planeNormal);
    // Plane and line are parallel
    if (std::abs(denom) < IMSTK_DOUBLE_EPS)
    {
        return false;
    }

    const double t1 = (a - p).dot(planeNormal);
    const double t2 = (a - q).dot(planeNormal);

    // Check if p and q lie on opposites side of the plane
    if ((t1 < 0.0 && t2 >= 0.0) || (t1 >= 0.0 && t2 < 0.0))
    {
        // \todo: Does planeNormal need to be normalized to get valid p + t1 * n, given division by denom
        //t1 /= denom;
        //t2 /= denom;
        const Vec3d uvw = baryCentric(p + t1 / denom * n, a, b, c);
        // Lastly check if the point on the plane p+t1*n is inside the triangle
        return (uvw[0] >= 0.0 && uvw[1] >= 0.0 && uvw[2] >= 0.0);
    }
    else
    {
        return false;
    }
}

///
/// \brief Returns the closest position to point on segment a-b and the case
/// type=0: x1 is the closest point
/// type=1: x2 is the closest point
/// type=2: closest point is on x1-x2
///
Vec3d closestPointOnSegment(const Vec3d& point, const Vec3d& x1, const Vec3d& x2, int& caseType);

///
/// \brief Returns the position closest to triangle a-b-c and the case <br>
/// type=0: a is the closest point <br>
/// type=1: b is the closest point <br>
/// type=2: c is the closest point <br>
/// type=3: closest point on ab <br>
/// type=4: closest point on bc <br>
/// type=5: closest point on ca <br>
/// type=6: closest point on face
///
Vec3d closestPointOnTriangle(const Vec3d& p, const Vec3d& a, const Vec3d& b, const Vec3d& c, int& caseType);

///
/// \brief Check if sphere and triangle intersect, with point direction contact info
/// \return true if intersecting, false if not
///
inline bool
testSphereToTriangle(const Vec3d& spherePt, const double sphereRadius,
                     const Vec3d& a, const Vec3d& b, const Vec3d& c,
                     Vec3d& sphereContactPt, Vec3d& sphereContactNormal, double& penetrationDepth)
{
    int         type = 0;
    const Vec3d closestPtOnTriangle = closestPointOnTriangle(spherePt, a, b, c, type);

    sphereContactNormal = (spherePt - closestPtOnTriangle);
    const double dist = sphereContactNormal.norm();
    penetrationDepth     = sphereRadius - dist;
    sphereContactNormal /= dist;
    sphereContactPt      = spherePt - sphereContactNormal * sphereRadius;
    return dist < sphereRadius;
}

///
/// \brief Check if sphere and triangle intersect, with contact info
/// If edge contact, edgeContact gives the indices of the two vertices in contact,
/// If point contact, pointContact gives the index of the vertex in contact
/// \return contact type, 0-no intersection, 1-edge contact, 2-face contact, 3-point contact
///
inline int
testSphereToTriangle(const Vec3d& spherePt, const double sphereRadius,
                     const Vec3i& tri, const Vec3d& a, const Vec3d& b, const Vec3d& c,
                     Vec3d& triangleContactPt,
                     Vec2i& edgeContact,
                     int& pointContact)
{
    int caseType = 0;
    triangleContactPt = closestPointOnTriangle(spherePt, a, b, c, caseType);
    const Vec3d  diff = spherePt - triangleContactPt;
    const double dist = diff.norm();

    // If intersecting
    if (dist <= sphereRadius)
    {
        switch (caseType)
        {
        case 0:
            pointContact = tri[0];
            return 3;
        case 1:
            pointContact = tri[1];
            return 3;
        case 2:
            pointContact = tri[2];
            return 3;
        case 3:
            edgeContact = { tri[0], tri[1] };
            return 1;
        case 4:
            edgeContact = { tri[1], tri[2] };
            return 1;
        case 5:
            edgeContact = { tri[2], tri[0] };
            return 1;
        case 6:
            return 2;
        default:
            return 0;
            break;
        }
        ;
    }
    else
    {
        return 0;
    }
}

///
/// \brief Tests if a point is inside a tetrahedron
///
inline bool
testPointToTetrahedron(const std::array<Vec3d, 4>& inputTetVerts, const Vec3d& p)
{
    const Vec4d bCoord = baryCentric(p, inputTetVerts[0], inputTetVerts[1], inputTetVerts[2], inputTetVerts[3]);

    constexpr const double eps = IMSTK_DOUBLE_EPS;
    if (bCoord[0] >= -eps
        && bCoord[1] >= -eps
        && bCoord[2] >= -eps
        && bCoord[3] >= -eps)
    {
        return true;
    }

    return false;
}

///
/// \brief Tests if the segment intersects any of the triangle faces of the tet
/// \todo Could be faster with SAT directly applied here
///
inline bool
testTetToSegment(
    const std::array<Vec3d, 4>& inputTetVerts,
    const Vec3d& x1, const Vec3d& x2)
{
    static int faces[4][3] = { { 0, 1, 2 }, { 1, 2, 3 }, { 0, 2, 3 }, { 0, 1, 3 } };
    for (int i = 0; i < 4; i++)
    {
        if (testSegmentTriangle(x1, x2, inputTetVerts[faces[i][0]], inputTetVerts[faces[i][1]], inputTetVerts[faces[i][2]]))
        {
            return true;
        }
    }

    // If either point lies inside the tetrahedron (handles completely inside case)
    if (testPointToTetrahedron(inputTetVerts, x1) || testPointToTetrahedron(inputTetVerts, x2))
    {
        return true;
    }

    return false;
}

///
/// \brief Tests if the segment intersects any of the triangle faces of the tet
/// Returns -1 when intersection face not present
/// Also returns intersection points
///
inline bool
testTetToSegment(
    const std::array<Vec3d, 4>& inputTetVerts,
    const Vec3d& x1, const Vec3d& x2,
    int& intersectionFace0, int& intersectionFace1,
    Vec3d& iPt0, Vec3d& iPt1)
{
    static int faces[4][3] = { { 0, 1, 2 }, { 1, 2, 3 }, { 0, 2, 3 }, { 0, 1, 3 } };
    bool       firstFound  = false;
    for (int i = 0; i < 4; i++)
    {
        const Vec3d& a = inputTetVerts[faces[i][0]];
        const Vec3d& b = inputTetVerts[faces[i][1]];
        const Vec3d& c = inputTetVerts[faces[i][2]];
        Vec3d        uvw;
        if (testSegmentTriangle(x1, x2, a, b, c, uvw))
        {
            if (firstFound)
            {
                intersectionFace1 = i;
                iPt1 = uvw[0] * a + uvw[1] * b + uvw[2] * c;
                return true;
            }
            else
            {
                intersectionFace0 = i;
                iPt0 = uvw[0] * a + uvw[1] * b + uvw[2] * c;

                // There could still be one intersection face, keep searching
                firstFound = true;
            }
        }
    }
    return firstFound;
}

///
/// \brief ray OBB intersection with intersection point
/// \param ray origin
/// \param ray direction
/// \param box to world
/// \param world to box
/// \param half length/extents
/// \param intersection points (first component gives entry point, second gives exit)
///
inline bool
testRayToObb(const Vec3d& rayOrigin, const Vec3d& rayDir,
             const Mat4d& worldToBox, Vec3d extents,
             Vec2d& iPt)
{
    // convert from world to box space
    const Vec3d rd = (worldToBox * Vec4d(rayDir[0], rayDir[1], rayDir[2], 0.0)).head<3>();
    const Vec3d ro = (worldToBox * Vec4d(rayOrigin[0], rayOrigin[1], rayOrigin[2], 1.0)).head<3>();

    // ray-box intersection in box space
    const Vec3d m = Vec3d(1.0, 1.0, 1.0).cwiseQuotient(rd);
    const Vec3d s = Vec3d((rd[0] < 0.0) ? 1.0 : -1.0,
        (rd[1] < 0.0) ? 1.0 : -1.0,
        (rd[2] < 0.0) ? 1.0 : -1.0);
    const Vec3d t1 = m.cwiseProduct(-ro + s.cwiseProduct(extents));
    const Vec3d t2 = m.cwiseProduct(-ro - s.cwiseProduct(extents));

    const double tN = std::max(std::max(t1[0], t1[1]), t1[2]);
    const double tF = std::min(std::min(t2[0], t2[1]), t2[2]);

    // Does not enter
    if (tN > tF || tF < 0.0)
    {
        return false;
    }
    iPt = Vec2d(tN, tF); // Parameterized along the ray

    return true;
}

///
/// \brief ray sphere intersection with intersection point
/// Real time collision detection 2004
///
inline bool
testRayToSphere(const Vec3d& rayOrigin, const Vec3d& rayDir,
                const Vec3d& spherePos, const double radius,
                Vec3d& iPt)
{
    const Vec3d m = rayOrigin - spherePos;
    double      b = m.dot(rayDir);
    double      c = m.dot(m) - radius * radius;

    // Exit if r’s origin outside s (c > 0) and r pointing away from s (b > 0)
    if (c > 0.0 && b > 0.0)
    {
        return false;
    }
    double discr = b * b - c;
    // A negative discriminant corresponds to ray missing sphere
    if (discr < 0.0)
    {
        return false;
    }
    // Ray now found to intersect sphere, compute smallest t value of intersection
    // If t is negative, ray started inside sphere so clamp t to zero
    double t = std::max(0.0, -b - std::sqrt(discr));
    iPt = rayOrigin + t * rayDir;
    return true;
}

///
/// \brief Ray plane intersection with intersection point
///
inline bool
testRayToPlane(const Vec3d& rayOrigin, const Vec3d& rayDir,
               const Vec3d& planePos, const Vec3d& planeNormal,
               Vec3d& iPt)
{
    const double denom = rayDir.dot(planeNormal);
    // Plane and ray are parallel
    if (std::abs(denom) < IMSTK_DOUBLE_EPS)
    {
        return false;
    }
    const double t = (planePos - rayOrigin).dot(planeNormal) / denom;
    // Ray points out from plane
    if (t <= 0.0)
    {
        return false;
    }
    iPt = rayOrigin + t * rayDir;
    return true;
}

///
/// \brief Compute closest distance from a point to a segment x1-x2
///
double pointSegmentClosestDistance(const Vec3d& point, const Vec3d& x1, const Vec3d& x2);

///
/// \brief Compute closest distance from a point to a triangle x1-x2-x3
///
double pointTriangleClosestDistance(const Vec3d& point, const Vec3d& x1, const Vec3d& x2, const Vec3d& x3);

///
/// \brief Given two triangles and their ids produce the vertex ids for edge-edge and
/// vertex-triangle contacts. eeContact data is always a-b ordered, vt data is a-b, but tv data is b-a.
/// \todo There is one edge case where the point of a triangle lies incident on the plane, ee should be
/// produced but fails. This may be too expensive to resolve?
/// \return int denoting which contact type it is -1=none, 0=ee, 1=vt, 2=tv
///
int triangleToTriangle(
    const Vec3i& tri_a, const Vec3i& tri_b,
    const Vec3d& p0_a, const Vec3d& p1_a, const Vec3d& p2_a,
    const Vec3d& p0_b, const Vec3d& p1_b, const Vec3d& p2_b,
    std::pair<Vec2i, Vec2i>& edgeContact,
    std::pair<int, Vec3i>& vertexTriangleContact,
    std::pair<Vec3i, int>& triangleVertexContact);

///
/// \brief Computes the closest point on two edges, reports midpoint
/// when colinear. From "Real Time Collision Detection"
/// \param first vertex of edge A
/// \param second vertex of edge A
/// \param first vertex of edge B
/// \param second vertex of edge B
/// \param closest point on edge A
/// \param closest point on edge B
/// \return case, 0 - closest point on edge, 1 - closest point on point of at least one edge
///
inline int
edgeToEdgeClosestPoints(
    const Vec3d& a0, const Vec3d& a1,
    const Vec3d& b0, const Vec3d& b1,
    Vec3d& ptA, Vec3d& ptB)
{
    double s = 0.0;
    double t = 0.0;
    int    caseType = 0;

    const Vec3d  d1 = a1 - a0;    // Direction vector of segment S1
    const Vec3d  d2 = b1 - b0;    // Direction vector of segment S2
    const Vec3d  r  = a0 - b0;
    const double a  = d1.dot(d1); // Squared length of segment S1, always nonnegative
    const double e  = d2.dot(d2); // Squared length of segment S2, always nonnegative
    const double f  = d2.dot(r);

    // Check if either or both segments degenerate into points
    if (a <= IMSTK_DOUBLE_EPS && e <= IMSTK_DOUBLE_EPS)
    {
        // Both segments degenerate into points
        s        = t = 0.0;
        ptA      = a0;
        ptB      = b0;
        caseType = 1;
        return caseType;
    }
    if (a <= IMSTK_DOUBLE_EPS)
    {
        // First segment degenerates into a point
        s = 0.0;
        t = f / e; // s = 0 => t = (b*s + f) / e = f / e
        t = std::min(std::max(t, 0.0), 1.0);
        caseType = 1;
    }
    else
    {
        const double c = d1.dot(r);
        if (e <= IMSTK_DOUBLE_EPS)
        {
            // Second segment degenerates into a point
            t = 0.0;
            s = std::min(std::max(-c / a, 0.0), 1.0); // t = 0 => s = (b*t - c) / a = -c / a
            caseType = 1;
        }
        else
        {
            // The general nondegenerate case starts here
            const double b     = d1.dot(d2);
            const double denom = a * e - b * b; // Always nonnegative
            // If segments not parallel, compute closest point on L1 to L2 and
            // clamp to segment S1. Else pick arbitrary s (here 0)
            if (denom != 0.0)
            {
                s = std::min(std::max((b * f - c * e) / denom, 0.0), 1.0);
            }
            else
            {
                s = 0.0;
                caseType = 1;
            }
            // Compute point on L2 closest to S1(s) using
            // t = Dot((P1 + D1*s) - P2,D2) / Dot(D2,D2) = (b*s + f) / e
            t = (b * s + f) / e;
            // If t in [0,1] done. Else clamp t, recompute s for the new value
            // of t using s = Dot((P2 + D2*t) - P1,D1) / Dot(D1,D1)= (t*b - c) / a
            // and clamp s to [0, 1]
            if (t < 0.0)
            {
                t = 0.0;
                s = std::min(std::max(-c / a, 0.0), 1.0);
                caseType = 1;
            }
            else if (t > 1.0)
            {
                t = 1.0;
                s = std::min(std::max((b - c) / a, 0.0), 1.0);
                caseType = 1;
            }
        }
    }
    ptA = a0 + d1 * s;
    ptB = b0 + d2 * t;

    return caseType;
}
} // namespace CollisionUtils
} // namespace imstk
