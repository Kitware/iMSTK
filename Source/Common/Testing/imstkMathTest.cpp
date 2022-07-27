/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "gtest/gtest.h"

#include "imstkMath.h"
#include "imstkTypes.h"

using namespace imstk;

TEST(imstkMathTest, TestBarycentricTriangle3d)
{
    const Vec3d bcCoord = imstk::baryCentric(
        Vec3d(0.0, 0.0, 0.0),
        Vec3d(1.0, 0.0, 0.0),
        Vec3d(0.0, 1.0, 0.0),
        Vec3d(0.0, 0.0, 1.0));
    EXPECT_NEAR(bcCoord[0], 1.0 / 3.0, IMSTK_DOUBLE_EPS);
    EXPECT_NEAR(bcCoord[1], 1.0 / 3.0, IMSTK_DOUBLE_EPS);
    EXPECT_NEAR(bcCoord[2], 1.0 / 3.0, IMSTK_DOUBLE_EPS);
    EXPECT_NEAR(bcCoord[0] + bcCoord[1] + bcCoord[2], 1.0, IMSTK_DOUBLE_EPS);
}

TEST(imstkMathTest, TestBarycentricTriangle2d)
{
    const Vec3d bcCoord = imstk::baryCentric(
        Vec2d(0.0, 0.0),
        Vec2d(-1.0, 0.0),
        Vec2d(1.0, 0.0),
        Vec2d(0.0, 1.0));
    EXPECT_NEAR(bcCoord[0], 1.0 / 2.0, IMSTK_DOUBLE_EPS);
    EXPECT_NEAR(bcCoord[1], 1.0 / 2.0, IMSTK_DOUBLE_EPS);
    EXPECT_NEAR(bcCoord[2], 0.0, IMSTK_DOUBLE_EPS);
    EXPECT_NEAR(bcCoord[0] + bcCoord[1] + bcCoord[2], 1.0, IMSTK_DOUBLE_EPS);
}

TEST(imstkMathTest, TestBarycentricLine3d)
{
    Vec2d bcCoord = imstk::baryCentric(
        Vec3d(0.0, 0.0, 0.0),
        Vec3d(-1.0, -1.0, -1.0),
        Vec3d(1.0, 1.0, 1.0));
    EXPECT_NEAR(bcCoord[0], 1.0 / 2.0, IMSTK_DOUBLE_EPS);
    EXPECT_NEAR(bcCoord[1], 1.0 / 2.0, IMSTK_DOUBLE_EPS);
    EXPECT_NEAR(bcCoord[0] + bcCoord[1], 1.0, IMSTK_DOUBLE_EPS);

    // Check a non-midpoint (closer to q)
    bcCoord = imstk::baryCentric(
        Vec3d(0.5, 0.5, 0.5),
        Vec3d(-1.0, -1.0, -1.0),
        Vec3d(1.0, 1.0, 1.0));
    EXPECT_NEAR(bcCoord[0], 0.25, IMSTK_DOUBLE_EPS);
    EXPECT_NEAR(bcCoord[1], 0.75, IMSTK_DOUBLE_EPS);
    EXPECT_NEAR(bcCoord[0] + bcCoord[1], 1.0, IMSTK_DOUBLE_EPS);

    // Check an extrapolated point (past q)
    bcCoord = imstk::baryCentric(
        Vec3d(1.5, 1.5, 1.5),
        Vec3d(-1.0, -1.0, -1.0),
        Vec3d(1.0, 1.0, 1.0));
    EXPECT_NEAR(bcCoord[0], -0.25, IMSTK_DOUBLE_EPS);
    EXPECT_NEAR(bcCoord[1], 1.25, IMSTK_DOUBLE_EPS);
    EXPECT_NEAR(bcCoord[0] + bcCoord[1], 1.0, IMSTK_DOUBLE_EPS);
}

TEST(imstkMathTest, TestBarycentricTetrahedron3d)
{
    const Vec4d bcCoord = imstk::baryCentric(
        Vec3d(0.0, 0.0, 0.0),
        Vec3d(1.0, 0.0, -1.0 / std::sqrt(2.0)),
        Vec3d(-1.0, 0.0, -1.0 / std::sqrt(2.0)),
        Vec3d(0.0, 1.0, 1.0 / std::sqrt(2.0)),
        Vec3d(0.0, -1.0, 1.0 / std::sqrt(2.0)));
    EXPECT_NEAR(bcCoord[0], 1.0 / 4.0, IMSTK_DOUBLE_EPS);
    EXPECT_NEAR(bcCoord[1], 1.0 / 4.0, IMSTK_DOUBLE_EPS);
    EXPECT_NEAR(bcCoord[2], 1.0 / 4.0, IMSTK_DOUBLE_EPS);
    EXPECT_NEAR(bcCoord[3], 1.0 / 4.0, IMSTK_DOUBLE_EPS);
    EXPECT_NEAR(bcCoord[0] + bcCoord[1] + bcCoord[2] + bcCoord[3], 1.0, IMSTK_DOUBLE_EPS);
}