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
#include "gmock/gmock.h"

#include "imstkMath.h"
#include "imstkCube.h"
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
    EXPECT_EQ(m_cube.getWidth(), 2);

    m_cube.setWidth(0.003);
    EXPECT_EQ(m_cube.getWidth(), 0.003);

    m_cube.setWidth(400000000);
    EXPECT_EQ(m_cube.getWidth(), 400000000);

    m_cube.setWidth(0);
    EXPECT_GT(m_cube.getWidth(), 0);

    m_cube.setWidth(-5);
    EXPECT_GT(m_cube.getWidth(), 0);
}

///
/// \brief TODO
///
TEST_F(imstkCubeTest, GetVolume)
{
    m_cube.setWidth(2);
    EXPECT_EQ(m_cube.getVolume(), 8);

    m_cube.setWidth(0.003);
    EXPECT_EQ(m_cube.getVolume(), 0.003 * 0.003 * 0.003);

    double w = 400000000;
    m_cube.setWidth(400000000);
    EXPECT_EQ(m_cube.getVolume(), w * w * w);
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
