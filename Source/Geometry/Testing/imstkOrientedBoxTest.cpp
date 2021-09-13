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
