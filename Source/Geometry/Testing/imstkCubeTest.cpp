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

#include "imstkCube.h"

#include <gtest/gtest.h>

using namespace imstk;

class imstkCubeTest : public ::testing::Test
{
protected:
    Cube m_cube;
};

///
/// \brief TODO
///
TEST_F(imstkCubeTest, SetGetWidth)
{
    m_cube.setWidth(2);
    EXPECT_DOUBLE_EQ(2, m_cube.getWidth());

    m_cube.setWidth(0.003);
    EXPECT_DOUBLE_EQ(0.003, m_cube.getWidth());

    m_cube.setWidth(400000000);
    EXPECT_DOUBLE_EQ(400000000, m_cube.getWidth());

    m_cube.setWidth(0);
    EXPECT_LT(0, m_cube.getWidth());

    m_cube.setWidth(-5);
    EXPECT_LT(0, m_cube.getWidth());
}

///
/// \brief TODO
///
TEST_F(imstkCubeTest, GetVolume)
{
    m_cube.setWidth(2);
    EXPECT_DOUBLE_EQ(8, m_cube.getVolume());

    m_cube.setWidth(0.003);
    EXPECT_DOUBLE_EQ(0.003 * 0.003 * 0.003, m_cube.getVolume());

    const double w = 400000000;
    m_cube.setWidth(400000000);
    EXPECT_DOUBLE_EQ(w * w * w, m_cube.getVolume());
}

///
/// \brief test the cube SDF evaluator
///
TEST_F(imstkCubeTest, GetFunctionValue)
{
    m_cube.setWidth(2.);
    m_cube.updatePostTransformData();

    EXPECT_DOUBLE_EQ(-1., m_cube.getFunctionValue(Vec3d(0.0, 0.0, 0.0)));
    EXPECT_DOUBLE_EQ(-0.5, m_cube.getFunctionValue(Vec3d(0.5, 0.0, 0.0)));
    EXPECT_DOUBLE_EQ(0.0, m_cube.getFunctionValue(Vec3d(1.0, 1.0, 1.0)));
    EXPECT_DOUBLE_EQ(9.0, m_cube.getFunctionValue(Vec3d(0.0, -10.0, 0.0)));

    m_cube.rotate(Vec3d(1.0, 0.0, 0.0), 0.5 * PI);
    m_cube.updatePostTransformData();

    EXPECT_DOUBLE_EQ(-1.0, m_cube.getFunctionValue(Vec3d(0.0, 0.0, 0.0)));
    EXPECT_DOUBLE_EQ(-0.5, m_cube.getFunctionValue(Vec3d(0.5, 0.0, 0.0)));
    EXPECT_NEAR(0.0, m_cube.getFunctionValue(Vec3d(1.0, 1.0, 1.0)), 1.0e-10);
    EXPECT_DOUBLE_EQ(9.0, m_cube.getFunctionValue(Vec3d(0.0, -10.0, 0.0)));

    m_cube.rotate(Vec3d(1.0, 0.0, 0.0), 0.25 * PI);
    m_cube.updatePostTransformData();

    EXPECT_DOUBLE_EQ(-1.0, m_cube.getFunctionValue(Vec3d(0.0, 0.0, 0.0)));
}

///
/// \brief TODO
///
int
imstkCubeTest(int argc, char* argv[])
{
    // Init Google Test & Mock
    ::testing::InitGoogleTest(&argc, argv);

    // Run tests with gtest
    return RUN_ALL_TESTS();
}
