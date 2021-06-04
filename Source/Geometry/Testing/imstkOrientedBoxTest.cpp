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

#include "imstkOrientedBox.h"

#include <gtest/gtest.h>

using namespace imstk;

class imstkOrientedBoxTest : public ::testing::Test
{
protected:
    OrientedBox m_box;
};

TEST_F(imstkOrientedBoxTest, SetGetWidth)
{
    m_box.setExtents(1.0, 1.0, 1.0);
    const Vec3d extents = m_box.getExtents();
    EXPECT_DOUBLE_EQ(1.0, extents[0]);
    EXPECT_DOUBLE_EQ(1.0, extents[1]);
    EXPECT_DOUBLE_EQ(1.0, extents[2]);
}

TEST_F(imstkOrientedBoxTest, GetVolume)
{
    m_box.setExtents(1.0, 1.0, 1.0);
    EXPECT_DOUBLE_EQ(8, m_box.getVolume());
}

TEST_F(imstkOrientedBoxTest, GetFunctionValue)
{
    m_box.setExtents(1.0, 1.0, 2.0);
    m_box.updatePostTransformData();

    EXPECT_DOUBLE_EQ(-1., m_box.getFunctionValue(Vec3d(0.0, 0.0, 0.0)));
    EXPECT_DOUBLE_EQ(-0.5, m_box.getFunctionValue(Vec3d(0.5, 0.0, 0.0)));
    EXPECT_DOUBLE_EQ(0.0, m_box.getFunctionValue(Vec3d(1.0, 1.0, 2.0)));
    EXPECT_DOUBLE_EQ(9.0, m_box.getFunctionValue(Vec3d(0.0, -10.0, 0.0)));

    m_box.rotate(Vec3d(1.0, 0.0, 0.0), 0.5 * PI);
    m_box.updatePostTransformData();

    EXPECT_DOUBLE_EQ(-1.0, m_box.getFunctionValue(Vec3d(0.0, 0.0, 0.0)));
    EXPECT_DOUBLE_EQ(-0.5, m_box.getFunctionValue(Vec3d(0.5, 0.0, 0.0)));
    EXPECT_DOUBLE_EQ(-0.5, m_box.getFunctionValue(Vec3d(0.0, -1.5, 0.0)));
}
