/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "gtest/gtest.h"

#include "imstkCollisionUtils.h"

namespace imstk
{
namespace CollisionUtils
{
TEST(imstkCollisionUtilsTest, RayToOrientedBoxTest)
{
    Vec2d tPt = Vec2d(-1.0, -1.0);
    // Hit
    EXPECT_TRUE(testRayToObb(
        Vec3d(1.0, 0.0, 0.0), Vec3d(-1.0, 0.0, 0.0),
        Mat4d::Identity(), Vec3d(0.5, 0.5, 0.5),
        tPt));
    EXPECT_NE(tPt[0], -1.0);
    EXPECT_NE(tPt[1], -1.0);

    // Miss
    EXPECT_FALSE(CollisionUtils::testRayToObb(
        Vec3d(1.0, 1.0, 0.0), Vec3d(-1.0, 0.0, 0.0),
        Mat4d::Identity(), Vec3d(0.5, 0.5, 0.5),
        tPt));

    // Inside box pointing out hit
    EXPECT_TRUE(CollisionUtils::testRayToObb(
        Vec3d(0.0, 0.0, 0.0), Vec3d(-1.0, 0.0, 0.0),
        Mat4d::Identity(), Vec3d(0.5, 0.5, 0.5),
        tPt));
}

TEST(imstkCollisionUtilsTest, RayToPlane)
{
    // Test ray start above/below the plane with it
    Vec3d iPt;
    EXPECT_FALSE(CollisionUtils::testRayToPlane(
        Vec3d(0.0, 0.01, 0.0), Vec3d(0.0, 1.0, 0.0),
        Vec3d::Zero(), Vec3d(0.0, 1.0, 0.0),
        iPt));
    EXPECT_TRUE(CollisionUtils::testRayToPlane(
        Vec3d(0.0, -0.01, 0.0), Vec3d(0.0, 1.0, 0.0),
        Vec3d::Zero(), Vec3d(0.0, 1.0, 0.0),
        iPt));

    // Test when the line is coplanar with the plane
    EXPECT_FALSE(CollisionUtils::testRayToPlane(
        Vec3d(0.0, 0.0, 0.0), Vec3d(1.0, 0.0, 0.0),
        Vec3d::Zero(), Vec3d(0.0, 1.0, 0.0),
        iPt));
}

TEST(imstkCollisionUtilsTest, RayToSphere)
{
    // \todo: RayToSphere does not report exit point

    // Test ray outside of sphere miss and hit
    Vec3d iPt;
    EXPECT_TRUE(CollisionUtils::testRayToSphere(
        Vec3d(1.0, 0.0, 0.0), Vec3d(-1.0, 0.0, 0.0),
        Vec3d::Zero(), 0.5,
        iPt));
    EXPECT_FALSE(CollisionUtils::testRayToSphere(
        Vec3d(1.0, 1.0, 0.0), Vec3d(-1.0, 0.0, 0.0),
        Vec3d::Zero(), 0.5,
        iPt));

    // Test ray inside the sphere hit
    EXPECT_TRUE(CollisionUtils::testRayToSphere(
        Vec3d(0.0, 0.0, 0.0), Vec3d(1.0, 0.0, 0.0),
        Vec3d::Zero(), 0.5,
        iPt));
}

TEST(imstkColliosUtilsTest, PlaneToSphere)
{
    const double eps    = 1e-10;
    Vec3d        planeP = Vec3d(0, 0, 0);
    Vec3d        planeN = Vec3d(0, 1, 0);

    Vec3d  sphereP = Vec3d(0, 3, 0);
    double sphereR = 2.0;

    Vec3d  planeContactP, planeContactN;
    Vec3d  sphereContactP, sphereContactN;
    double depth;

    // Outside
    EXPECT_FALSE(testPlaneToSphere(planeP, planeN, sphereP, sphereR,
        planeContactP, planeContactN, sphereContactP, sphereContactN, depth));
    EXPECT_FALSE(testPlaneToSphere(planeP, planeN, sphereP, sphereR));

    // Glancing
    sphereP = Vec3d(0, 2, 0);

    EXPECT_FALSE(testPlaneToSphere(planeP, planeN, sphereP, sphereR,
        planeContactP, planeContactN, sphereContactP, sphereContactN, depth));
    EXPECT_FALSE(testPlaneToSphere(planeP, planeN, sphereP, sphereR));

    // A little inside
    sphereP = Vec3d(0, 1, 0);

    EXPECT_TRUE(testPlaneToSphere(planeP, planeN, sphereP, sphereR,
        planeContactP, planeContactN, sphereContactP, sphereContactN, depth));
    EXPECT_TRUE(testPlaneToSphere(planeP, planeN, sphereP, sphereR));
    EXPECT_DOUBLE_EQ(depth, 1);
    EXPECT_TRUE(sphereContactP.isApprox(Vec3d(0, -1, 0)));
    EXPECT_TRUE(planeContactP.isApprox(Vec3d(0, 0, 0)));

    // Edge Case
    sphereP = Vec3d(0, 0, 0);
    EXPECT_TRUE(testPlaneToSphere(planeP, planeN, sphereP, sphereR,
        planeContactP, planeContactN, sphereContactP, sphereContactN, depth));
    EXPECT_TRUE(testPlaneToSphere(planeP, planeN, sphereP, sphereR));
    EXPECT_DOUBLE_EQ(depth, 2);
    EXPECT_TRUE(sphereContactP.isApprox(Vec3d(0, -2, 0)));
    EXPECT_TRUE(planeContactP.isApprox(Vec3d(0, 0, 0)));

    // Plane above center
    sphereP = Vec3d(0, -1, 0);
    EXPECT_TRUE(testPlaneToSphere(planeP, planeN, sphereP, sphereR,
        planeContactP, planeContactN, sphereContactP, sphereContactN, depth));
    EXPECT_TRUE(testPlaneToSphere(planeP, planeN, sphereP, sphereR));
    EXPECT_DOUBLE_EQ(depth, 3);
    EXPECT_TRUE(sphereContactP.isApprox(Vec3d(0, -3, 0)));
    EXPECT_TRUE(planeContactP.isApprox(Vec3d(0, 0, 0)));

    // Sphere fully under plane
    sphereP = Vec3d(0, -2, 0);
    EXPECT_TRUE(testPlaneToSphere(planeP, planeN, sphereP, sphereR,
        planeContactP, planeContactN, sphereContactP, sphereContactN, depth));
    EXPECT_TRUE(testPlaneToSphere(planeP, planeN, sphereP, sphereR));
    EXPECT_DOUBLE_EQ(depth, 4);
    EXPECT_TRUE(sphereContactP.isApprox(Vec3d(0, -4, 0))) << sphereContactP.transpose();
    EXPECT_TRUE(planeContactP.isApprox(Vec3d(0, 0, 0))) << planeContactP.transpose();
}

TEST(imstkCollisionUtilsTest, ClosestPointOnTriangleTest)
{
    // Triangle abc
    const Vec3d& a = Vec3d(1.0, 0.0, -1.0);
    const Vec3d& b = Vec3d(-1.0, 0.0, -1.0);
    const Vec3d& c = Vec3d(0.0, 0.0, 1.0);

    int caseType = -1;

    // Test a
    // Point pA is closest to a
    const Vec3d& pA = Vec3d(1.1, 0.0, -1.1);

    const Vec3d trianglePointA = closestPointOnTriangle(pA, a, b, c, caseType);

    ASSERT_EQ(0, caseType);
    EXPECT_TRUE(trianglePointA.isApprox(a));

    // Test b
    // Point pB is closest to b
    const Vec3d& pB = Vec3d(-1.1, 0.0, -1.1);
    const Vec3d  trianglePointB = closestPointOnTriangle(pB, a, b, c, caseType);
    ASSERT_EQ(1, caseType);
    EXPECT_TRUE(trianglePointB.isApprox(b));

    // Test c
    // Point pC is closest to b
    const Vec3d& pC = Vec3d(0.0, 0.0, 1.1);
    const Vec3d  trianglePointC = closestPointOnTriangle(pC, a, b, c, caseType);
    ASSERT_EQ(2, caseType);
    EXPECT_TRUE(trianglePointC.isApprox(c));

    // Test edge AB
    // Point pAB is closest edge AB
    const Vec3d& pAB = Vec3d(0.0, 0.0, -1.1);
    const Vec3d  trianglePointAB = closestPointOnTriangle(pAB, a, b, c, caseType);
    ASSERT_EQ(3, caseType);
    EXPECT_TRUE(trianglePointAB.isApprox(Vec3d(0.0, 0.0, -1.0)));

    // Test edge BC
    // Point pBC is closest edge BC
    const Vec3d& pBC = Vec3d(-1.0, 0.0, 0.0);
    const Vec3d  trianglePointBC = closestPointOnTriangle(pBC, a, b, c, caseType);
    ASSERT_EQ(4, caseType);

    // Test edge AC
    // Point pAC is closest edge AC
    const Vec3d& pAC = Vec3d(1.0, 0.0, 0.0);
    const Vec3d  trianglePointAC = closestPointOnTriangle(pAC, a, b, c, caseType);
    ASSERT_EQ(5, caseType);

    // Test inside triangle
    // Point pAC is closest edge AC
    const Vec3d& pCenter = Vec3d(0.0, 0.0, 0.0);
    const Vec3d  trianglePointCenter = closestPointOnTriangle(pCenter, a, b, c, caseType);
    ASSERT_EQ(6, caseType);
    EXPECT_TRUE(trianglePointCenter.isApprox(pCenter));
}
} // namespace CollisionUtils
} // namespace imstk
