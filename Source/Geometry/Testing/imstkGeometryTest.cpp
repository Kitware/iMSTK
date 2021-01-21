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

#include "imstkPlane.h"

using namespace imstk;

///
/// \brief TODO
///
class imstkGeometryTest : public ::testing::Test
{
protected:
    Plane m_geometry; // Can't use imstk::Geometry since pure virtual. Should use google mock class.
};

///
/// \brief TODO
///
TEST_F(imstkGeometryTest, GetSetScaling)
{
    m_geometry.setScaling(Vec3d(2.0, 2.0, 2.0));
    EXPECT_EQ(m_geometry.getScaling(), Vec3d(2.0, 2.0, 2.0));

    m_geometry.setScaling(Vec3d(0.003, 0.003, 0.003));
    EXPECT_EQ(m_geometry.getScaling(), Vec3d(0.003, 0.003, 0.003));

    m_geometry.setScaling(Vec3d(400000000.0, 400000000.0, 400000000.0));
    EXPECT_EQ(m_geometry.getScaling(), Vec3d(400000000.0, 400000000.0, 400000000.0));
}

///
/// \brief TODO
///
TEST_F(imstkGeometryTest, GetSetTranslation)
{
    auto p1 = Vec3d(12.0, 0.0005, -400000.0);
    auto p2 = Vec3d(-500.0, 30.0, 0.23);

    m_geometry.setTranslation(p1);
    EXPECT_EQ(m_geometry.getTranslation(), p1);

    m_geometry.setTranslation(p2);
    EXPECT_EQ(m_geometry.getTranslation(), p2);

    m_geometry.setTranslation(p1[0], p1[1], p1[2]);
    EXPECT_EQ(m_geometry.getTranslation(), p1);

    m_geometry.setTranslation(p2[0], p2[1], p2[2]);
    EXPECT_EQ(m_geometry.getTranslation(), p2);
}

///
/// \brief TODO
///
TEST_F(imstkGeometryTest, GetSetRotation1)
{
    // NOTE: '==' not defined for Eigen::Quaternion, using 'isApprox'.
    // See https://forum.kde.org/viewtopic.php?f=74&t=118598

    // Rotation is normalized so comparing pre/post requires starting orientation
    // and angles < 360deg. As well as normalized axes.
    const Quatd q1 = Quatd(Rotd(4.1, Vec3d(12.0, 0.0, -0.5).normalized()));
    m_geometry.setRotation(q1);
    EXPECT_TRUE(Quatd(m_geometry.getRotation()).isApprox(q1));
}

///
/// \brief TODO
///
TEST_F(imstkGeometryTest, GetSetRotation2)
{
    // NOTE: '==' not defined for Eigen::Quaternion, using 'isApprox'.
    // See https://forum.kde.org/viewtopic.php?f=74&t=118598

    // Rotation is normalized so comparing pre/post requires starting orientation
    // and angles < 360deg. As well as normalized axes.
    const Mat3d mat2 = Mat3d(Rotd(0.43, Vec3d(4000.0, -1.0, 0.0).normalized()));
    m_geometry.setRotation(mat2);
    EXPECT_TRUE(m_geometry.getRotation().isApprox(mat2));
}

///
/// \brief TODO
///
TEST_F(imstkGeometryTest, GetSetRotation3)
{
    // NOTE: '==' not defined for Eigen::Quaternion, using 'isApprox'.
    // See https://forum.kde.org/viewtopic.php?f=74&t=118598

    // Rotation is normalized so comparing pre/post requires starting orientation
    // and angles < 360deg. As well as normalized axes.
    const double angle = 1.57;
    const Vec3d  axes  = Vec3d(-0.0, 100.0, 2000000.0).normalized();
    const Mat3d  mat3  = Mat3d(Rotd(angle, axes));
    m_geometry.setRotation(axes, angle);
    EXPECT_TRUE(m_geometry.getRotation().isApprox(mat3));
}

///
/// \brief TODO
///
int
imstkGeometryTest(int argc, char* argv[])
{
    // Init Google Test & Mock
    ::testing::InitGoogleTest(&argc, argv);

    // Run tests with gtest
    return RUN_ALL_TESTS();
}
