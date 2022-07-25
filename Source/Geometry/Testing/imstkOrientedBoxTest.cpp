/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkOrientedBox.h"

#include <gtest/gtest.h>

using namespace imstk;

TEST(imstkOrientedBoxTest, SetGetWidth)
{
    OrientedBox box;
    box.setExtents(1.0, 1.0, 1.0);
    const Vec3d extents = box.getExtents();
    EXPECT_DOUBLE_EQ(1.0, extents[0]);
    EXPECT_DOUBLE_EQ(1.0, extents[1]);
    EXPECT_DOUBLE_EQ(1.0, extents[2]);
}

TEST(imstkOrientedBoxTest, GetVolume)
{
    OrientedBox box;
    box.setExtents(1.0, 1.0, 1.0);
    EXPECT_DOUBLE_EQ(8, box.getVolume());
}

TEST(imstkOrientedBoxTest, GetFunctionValue)
{
    OrientedBox box;
    box.setExtents(1.0, 1.0, 2.0);
    box.updatePostTransformData();

    EXPECT_DOUBLE_EQ(-1., box.getFunctionValue(Vec3d(0.0, 0.0, 0.0)));
    EXPECT_DOUBLE_EQ(-0.5, box.getFunctionValue(Vec3d(0.5, 0.0, 0.0)));
    EXPECT_DOUBLE_EQ(0.0, box.getFunctionValue(Vec3d(1.0, 1.0, 2.0)));
    EXPECT_DOUBLE_EQ(9.0, box.getFunctionValue(Vec3d(0.0, -10.0, 0.0)));

    box.rotate(Vec3d(1.0, 0.0, 0.0), 0.5 * PI);
    box.updatePostTransformData();

    EXPECT_DOUBLE_EQ(-1.0, box.getFunctionValue(Vec3d(0.0, 0.0, 0.0)));
    EXPECT_DOUBLE_EQ(-0.5, box.getFunctionValue(Vec3d(0.5, 0.0, 0.0)));
    EXPECT_DOUBLE_EQ(-0.5, box.getFunctionValue(Vec3d(0.0, -1.5, 0.0)));
}
