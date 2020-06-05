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
#include "imstkPlane.h"
using namespace imstk;

///
/// \brief TODO
///
class imstkPlaneTest : public ::testing::Test
{
protected:
    Plane m_plane;
};

///
/// \brief TODO
///
TEST_F(imstkPlaneTest, SetGetWidth)
{
    m_plane.setWidth(2);
    EXPECT_EQ(m_plane.getWidth(), 2);

    m_plane.setWidth(0.003);
    EXPECT_EQ(m_plane.getWidth(), 0.003);

    m_plane.setWidth(400000000);
    EXPECT_EQ(m_plane.getWidth(), 400000000);

    m_plane.setWidth(0);
    EXPECT_GT(m_plane.getWidth(), 0);

    m_plane.setWidth(-5);
    EXPECT_GT(m_plane.getWidth(), 0);
}

///
/// \brief TODO
///
TEST_F(imstkPlaneTest, SetGetNormal)
{
    Vec3d n1 = Vec3d(0.2, -0.3, 0.9);
    Vec3d n2 = Vec3d(0.003, -0.001, 0.002);
    Vec3d n3 = Vec3d(400000000, -500000000, 600000000);

    m_plane.setNormal(n1);
    EXPECT_TRUE(m_plane.getNormal().isApprox(n1.normalized()));

    m_plane.setNormal(n2);
    EXPECT_TRUE(m_plane.getNormal().isApprox(n2.normalized()));

    m_plane.setNormal(n3);
    EXPECT_TRUE(m_plane.getNormal().isApprox(n3.normalized()));

    m_plane.setNormal(0, 0, 0);
    EXPECT_FALSE(m_plane.getNormal().isApprox(Vec3d(0, 0, 0)));
}

///
/// \brief TODO
///
TEST_F(imstkPlaneTest, GetVolume)
{
    EXPECT_EQ(m_plane.getVolume(), 0);
}

///
/// \brief TODO
///
int
imstkPlaneTest(int argc, char* argv[])
{
    // Init Google Test & Mock
    ::testing::InitGoogleTest(&argc, argv);

    // Run tests with gtest
    return RUN_ALL_TESTS();
}
