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

#include "imstkMath.h"
#include "imstkTypes.h"

using namespace imstk;

TEST(imstkMathTest, TestBarycentricTriangle3d)
{
    const Vec3d bcCoord = imstk::baryCentric(
        Vec3d(0.0, 0.0, 0.0),
        Vec3d(1.0, 0.0, 0.0),
        Vec3d(0.0, 1.0, 0.0),
        Vec3d(0.0, 0.0, 1.0));
    EXPECT_NEAR(bcCoord[0], 1.0 / 3.0, IMSTK_DOUBLE_EPS);
    EXPECT_NEAR(bcCoord[1], 1.0 / 3.0, IMSTK_DOUBLE_EPS);
    EXPECT_NEAR(bcCoord[2], 1.0 / 3.0, IMSTK_DOUBLE_EPS);
    EXPECT_NEAR(bcCoord[0] + bcCoord[1] + bcCoord[2], 1.0, IMSTK_DOUBLE_EPS);
}

TEST(imstkMathTest, TestBarycentricTriangle2d)
{
    const Vec3d bcCoord = imstk::baryCentric(
        Vec2d(0.0, 0.0),
        Vec2d(-1.0, 0.0),
        Vec2d(1.0, 0.0),
        Vec2d(0.0, 1.0));
    EXPECT_NEAR(bcCoord[0], 1.0 / 2.0, IMSTK_DOUBLE_EPS);
    EXPECT_NEAR(bcCoord[1], 1.0 / 2.0, IMSTK_DOUBLE_EPS);
    EXPECT_NEAR(bcCoord[2], 0.0, IMSTK_DOUBLE_EPS);
    EXPECT_NEAR(bcCoord[0] + bcCoord[1] + bcCoord[2], 1.0, IMSTK_DOUBLE_EPS);
}

TEST(imstkMathTest, TestBarycentricLine3d)
{
    const Vec2d bcCoord = imstk::baryCentric(
        Vec3d(0.0, 0.0, 0.0),
        Vec3d(-1.0, -1.0, -1.0),
        Vec3d(1.0, 1.0, 1.0));
    EXPECT_NEAR(bcCoord[0], 1.0 / 2.0, IMSTK_DOUBLE_EPS);
    EXPECT_NEAR(bcCoord[1], 1.0 / 2.0, IMSTK_DOUBLE_EPS);
    EXPECT_NEAR(bcCoord[0] + bcCoord[1], 1.0, IMSTK_DOUBLE_EPS);
}

TEST(imstkMathTest, TestBarycentricTetrahedron3d)
{
    const Vec4d bcCoord = imstk::baryCentric(
        Vec3d(0.0, 0.0, 0.0),
        Vec3d(1.0, 0.0, -1.0 / std::sqrt(2.0)),
        Vec3d(-1.0, 0.0, -1.0 / std::sqrt(2.0)),
        Vec3d(0.0, 1.0, 1.0 / std::sqrt(2.0)),
        Vec3d(0.0, -1.0, 1.0 / std::sqrt(2.0)));
    EXPECT_NEAR(bcCoord[0], 1.0 / 4.0, IMSTK_DOUBLE_EPS);
    EXPECT_NEAR(bcCoord[1], 1.0 / 4.0, IMSTK_DOUBLE_EPS);
    EXPECT_NEAR(bcCoord[2], 1.0 / 4.0, IMSTK_DOUBLE_EPS);
    EXPECT_NEAR(bcCoord[3], 1.0 / 4.0, IMSTK_DOUBLE_EPS);
    EXPECT_NEAR(bcCoord[0] + bcCoord[1] + bcCoord[2] + bcCoord[3], 1.0, IMSTK_DOUBLE_EPS);
}