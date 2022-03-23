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
