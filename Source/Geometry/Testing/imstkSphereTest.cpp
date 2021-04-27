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

///
/// \brief TODO
///
class imstkSphereTest : public ::testing::Test
{
protected:
    Sphere m_sphere;
};

///
/// \brief TODO
///
TEST_F(imstkSphereTest, SetGetRadius)
{
    m_sphere.setRadius(2);
    EXPECT_EQ(m_sphere.getRadius(), 2);

    m_sphere.setRadius(0.003);
    EXPECT_EQ(m_sphere.getRadius(), 0.003);

    m_sphere.setRadius(400000000);
    EXPECT_EQ(m_sphere.getRadius(), 400000000);

    m_sphere.setRadius(0);
    EXPECT_GT(m_sphere.getRadius(), 0);

    m_sphere.setRadius(-5);
    EXPECT_GT(m_sphere.getRadius(), 0);
}

TEST_F(imstkSphereTest, GetVolume)
{
    m_sphere.setRadius(2);
    EXPECT_EQ(m_sphere.getVolume(), 4.0 / 3.0 * 8 * PI);

    m_sphere.setRadius(0.003);
    EXPECT_EQ(m_sphere.getVolume(), 4.0 / 3.0 * PI * 0.003 * 0.003 * 0.003);

    double r = 400000000;
    m_sphere.setRadius(400000000);
    EXPECT_EQ(m_sphere.getVolume(), 4.0 / 3.0 * PI * r * r * r);
}

///
/// \brief test the sphere SDF evaluator
///
TEST_F(imstkSphereTest, GetFunctionValue)
{
    m_sphere.setRadius(20.);
    m_sphere.updatePostTransformData();

    EXPECT_EQ(m_sphere.getFunctionValue(Vec3d(0., 0., 0.)), -20.);
    EXPECT_EQ(m_sphere.getFunctionValue(Vec3d(5., 0., 0.)), -15);
    EXPECT_EQ(m_sphere.getFunctionValue(Vec3d(1., 1., 1.)), -20.0 + std::sqrt(3));
    EXPECT_EQ(m_sphere.getFunctionValue(Vec3d(0., 20., 0.)), 0.);
    EXPECT_EQ(m_sphere.getFunctionValue(Vec3d(0., 0., 50.)), 30.);
}

///
/// \brief TODO
///
int
imstkSphereTest(int argc, char* argv[])
{
    // Init Google Test & Mock
    ::testing::InitGoogleTest(&argc, argv);

    // Run tests with gtest
    return RUN_ALL_TESTS();
}
