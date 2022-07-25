/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCapsule.h"

#include <gtest/gtest.h>

using namespace imstk;

TEST(imstkCapsuleTest, SetGetRadius)
{
   Capsule capsule;

   capsule.setRadius(2);
   EXPECT_DOUBLE_EQ(2, capsule.getRadius());

   capsule.setRadius(9000000);
   EXPECT_DOUBLE_EQ(9000000, capsule.getRadius());

   capsule.setRadius(0.0000002);
   EXPECT_DOUBLE_EQ(0.0000002, capsule.getRadius());

   EXPECT_DEATH(capsule.setRadius(0.0), "Capsule::setRadius error: radius should be positive.");
   EXPECT_DEATH(capsule.setRadius(-50.0), "Capsule::setRadius error: radius should be positive.");
}

TEST(imstkCapsuleTest, SetGetLength)
{
   Capsule capsule;

   capsule.setLength(10);
   EXPECT_DOUBLE_EQ(10, capsule.getLength());

   capsule.setLength(0.000004);
   EXPECT_DOUBLE_EQ(0.000004, capsule.getLength());

   capsule.setLength(1000000000);
   EXPECT_DOUBLE_EQ(1000000000, capsule.getLength());

   EXPECT_DEATH(capsule.setLength(-10.0), "Capsule::setLength error: length should be positive.");
}

TEST(imstkCapsuleTest, GetVolume)
{
   Capsule capsule;

   capsule.setLength(2.0);
   capsule.setRadius(1.0);
   EXPECT_DOUBLE_EQ(PI * (10.0 / 3.0), capsule.getVolume());

   capsule.setLength(20000.0);
   capsule.setRadius(10000000.0);
   EXPECT_DOUBLE_EQ(PI * 1.0e14 * (40060000.0 / 3.0), capsule.getVolume());
}

TEST(imstkCapsuleTest, GetFunctionalValue)
{
   Capsule capsule;

   capsule.setLength(2.0);
   capsule.setRadius(1.0);
   capsule.updatePostTransformData();

   EXPECT_DOUBLE_EQ(0.0, capsule.getFunctionValue(Vec3d(1.0, 0.0, 0.0)));
   EXPECT_DOUBLE_EQ(0.0, capsule.getFunctionValue(Vec3d(0.0, 0.0, 1.0)));
   EXPECT_DOUBLE_EQ(0.0, capsule.getFunctionValue(Vec3d(0.0, 2.0, 0.0)));

   EXPECT_DOUBLE_EQ(0.0, capsule.getFunctionValue(Vec3d(-1.0, 0.0, 0.0)));
   EXPECT_DOUBLE_EQ(0.0, capsule.getFunctionValue(Vec3d(0.0, 0.0, -1.0)));
   EXPECT_DOUBLE_EQ(0.0, capsule.getFunctionValue(Vec3d(0.0, -2.0, 0.0)));

   EXPECT_DOUBLE_EQ(1.0, capsule.getFunctionValue(Vec3d(2.0, 0.0, 0.0)));
   EXPECT_DOUBLE_EQ(1.0, capsule.getFunctionValue(Vec3d(0.0, 0.0, 2.0)));
   EXPECT_DOUBLE_EQ(1.0, capsule.getFunctionValue(Vec3d(0.0, 3.0, 0.0)));

   EXPECT_DOUBLE_EQ(1.0, capsule.getFunctionValue(Vec3d(-2.0, 0.0, 0.0)));
   EXPECT_DOUBLE_EQ(1.0, capsule.getFunctionValue(Vec3d(0.0, 0.0, -2.0)));
   EXPECT_DOUBLE_EQ(1.0, capsule.getFunctionValue(Vec3d(0.0, -3.0, 0.0)));

   EXPECT_DOUBLE_EQ(-0.5, capsule.getFunctionValue(Vec3d(0.5, 0.0, 0.0)));
   EXPECT_DOUBLE_EQ(-0.5, capsule.getFunctionValue(Vec3d(0.0, 0.0, 0.5)));
   EXPECT_DOUBLE_EQ(-1.0, capsule.getFunctionValue(Vec3d(0.0, 0.0, 0.0)));
}
