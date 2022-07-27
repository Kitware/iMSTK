/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkSphere.h"

#include <gtest/gtest.h>

using namespace imstk;

TEST(imstkSphereTest, SetGetRadius)
{
    Sphere sphere;

    sphere.setRadius(2);
    EXPECT_DOUBLE_EQ(2, sphere.getRadius());

    sphere.setRadius(0.003);
    EXPECT_DOUBLE_EQ(0.003, sphere.getRadius());

    sphere.setRadius(400000000);
    EXPECT_DOUBLE_EQ(400000000, sphere.getRadius());

    sphere.setRadius(0);
    EXPECT_LT(0, sphere.getRadius());

    sphere.setRadius(-5);
    EXPECT_LT(0, sphere.getRadius());
}

TEST(imstkSphereTest, GetVolume)
{
    Sphere sphere;
    sphere.setRadius(2);
    EXPECT_DOUBLE_EQ(4.0 / 3.0 * 8 * PI, sphere.getVolume());

    sphere.setRadius(0.003);
    EXPECT_DOUBLE_EQ(4.0 / 3.0 * PI * 0.003 * 0.003 * 0.003, sphere.getVolume());

    double r = 400000000;
    sphere.setRadius(400000000);
    EXPECT_DOUBLE_EQ(4.0 / 3.0 * PI * r * r * r, sphere.getVolume());
}

TEST(imstkSphereTest, GetFunctionValue)
{
    Sphere sphere;
    sphere.setRadius(20.);
    sphere.updatePostTransformData();

    EXPECT_DOUBLE_EQ(-20.0, sphere.getFunctionValue(Vec3d(0.0, 0.0, 0.0)));
    EXPECT_DOUBLE_EQ(-15.0, sphere.getFunctionValue(Vec3d(5.0, 0.0, 0.0)));
    EXPECT_DOUBLE_EQ(-20.0 + std::sqrt(3), sphere.getFunctionValue(Vec3d(1.0, 1.0, 1.0)));
    EXPECT_DOUBLE_EQ(0.0, sphere.getFunctionValue(Vec3d(0.0, 20.0, 0.0)));
    EXPECT_DOUBLE_EQ(30.0, sphere.getFunctionValue(Vec3d(0.0, 0.0, 50.0)));

    sphere.rotate(Vec3d(1.0, 1.0, 0.0), 0.1 * PI);
    sphere.updatePostTransformData();

    EXPECT_DOUBLE_EQ(-20.0, sphere.getFunctionValue(Vec3d(0.0, 0.0, 0.0)));
    EXPECT_DOUBLE_EQ(-15.0, sphere.getFunctionValue(Vec3d(5.0, 0.0, 0.0)));
    EXPECT_DOUBLE_EQ(-20.0 + std::sqrt(3), sphere.getFunctionValue(Vec3d(1.0, 1.0, 1.0)));
    EXPECT_NEAR(0., sphere.getFunctionValue(Vec3d(0.0, 20., 0.0)), 1.0e-10);
    EXPECT_DOUBLE_EQ(30.0, sphere.getFunctionValue(Vec3d(0.0, 0.0, 50.0)));
}
