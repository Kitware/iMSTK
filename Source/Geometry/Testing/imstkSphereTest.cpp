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

#include "gtest/gtest.h"

#include "imstkSphere.h"

using namespace imstk;

class imstkSphereTest : public ::testing::Test
{
protected:
    Sphere m_sphere;
};

TEST_F(imstkSphereTest, SetGetRadius)
{
    m_sphere.setRadius(2);
    EXPECT_DOUBLE_EQ(2, m_sphere.getRadius());

    m_sphere.setRadius(0.003);
    EXPECT_DOUBLE_EQ(0.003, m_sphere.getRadius());

    m_sphere.setRadius(400000000);
    EXPECT_DOUBLE_EQ(400000000, m_sphere.getRadius());

    m_sphere.setRadius(0);
    EXPECT_LT(0, m_sphere.getRadius());

    m_sphere.setRadius(-5);
    EXPECT_LT(0, m_sphere.getRadius());
}

TEST_F(imstkSphereTest, GetVolume)
{
    m_sphere.setRadius(2);
    EXPECT_DOUBLE_EQ(4.0 / 3.0 * 8 * PI, m_sphere.getVolume());

    m_sphere.setRadius(0.003);
    EXPECT_DOUBLE_EQ(4.0 / 3.0 * PI * 0.003 * 0.003 * 0.003, m_sphere.getVolume());

    double r = 400000000;
    m_sphere.setRadius(400000000);
    EXPECT_DOUBLE_EQ(4.0 / 3.0 * PI * r * r * r, m_sphere.getVolume());
}

TEST_F(imstkSphereTest, GetFunctionValue)
{
    m_sphere.setRadius(20.);
    m_sphere.updatePostTransformData();

    EXPECT_DOUBLE_EQ(-20.0, m_sphere.getFunctionValue(Vec3d(0.0, 0.0, 0.0)));
    EXPECT_DOUBLE_EQ(-15.0, m_sphere.getFunctionValue(Vec3d(5.0, 0.0, 0.0)));
    EXPECT_DOUBLE_EQ(-20.0 + std::sqrt(3), m_sphere.getFunctionValue(Vec3d(1.0, 1.0, 1.0)));
    EXPECT_DOUBLE_EQ(0.0, m_sphere.getFunctionValue(Vec3d(0.0, 20.0, 0.0)));
    EXPECT_DOUBLE_EQ(30.0, m_sphere.getFunctionValue(Vec3d(0.0, 0.0, 50.0)));

    m_sphere.rotate(Vec3d(1.0, 1.0, 0.0), 0.1 * PI);
    m_sphere.updatePostTransformData();

    EXPECT_DOUBLE_EQ(-20.0, m_sphere.getFunctionValue(Vec3d(0.0, 0.0, 0.0)));
    EXPECT_DOUBLE_EQ(-15.0, m_sphere.getFunctionValue(Vec3d(5.0, 0.0, 0.0)));
    EXPECT_DOUBLE_EQ(-20.0 + std::sqrt(3), m_sphere.getFunctionValue(Vec3d(1.0, 1.0, 1.0)));
    EXPECT_NEAR(0., m_sphere.getFunctionValue(Vec3d(0.0, 20., 0.0)), 1.0e-10);
    EXPECT_DOUBLE_EQ(30.0, m_sphere.getFunctionValue(Vec3d(0.0, 0.0, 50.0)));
}
