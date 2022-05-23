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

#include "imstkCollisionUtils.h"

namespace imstk
{
namespace CollisionUtils
{
TEST(imstkCollisionUtilsTest, RayToOrientedBoxTest)
{
    Vec2d tPt = Vec2d(-1.0, -1.0);
    // Hit
    EXPECT_TRUE(testRayToObb(
        Vec3d(1.0, 0.0, 0.0), Vec3d(-1.0, 0.0, 0.0),
        Mat4d::Identity(), Vec3d(0.5, 0.5, 0.5),
        tPt));
    EXPECT_NE(tPt[0], -1.0);
    EXPECT_NE(tPt[1], -1.0);

    // Miss
    EXPECT_FALSE(CollisionUtils::testRayToObb(
        Vec3d(1.0, 1.0, 0.0), Vec3d(-1.0, 0.0, 0.0),
        Mat4d::Identity(), Vec3d(0.5, 0.5, 0.5),
        tPt));

    // Inside box pointing out hit
    EXPECT_TRUE(CollisionUtils::testRayToObb(
        Vec3d(0.0, 0.0, 0.0), Vec3d(-1.0, 0.0, 0.0),
        Mat4d::Identity(), Vec3d(0.5, 0.5, 0.5),
        tPt));
}

TEST(imstkCollisionUtilsTest, RayToPlane)
{
    // Test ray start above/below the plane with it
    Vec3d iPt;
    EXPECT_FALSE(CollisionUtils::testRayToPlane(
        Vec3d(0.0, 0.01, 0.0), Vec3d(0.0, 1.0, 0.0),
        Vec3d::Zero(), Vec3d(0.0, 1.0, 0.0),
        iPt));
    EXPECT_TRUE(CollisionUtils::testRayToPlane(
        Vec3d(0.0, -0.01, 0.0), Vec3d(0.0, 1.0, 0.0),
        Vec3d::Zero(), Vec3d(0.0, 1.0, 0.0),
        iPt));

    // Test when the line is coplanar with the plane
    EXPECT_FALSE(CollisionUtils::testRayToPlane(
        Vec3d(0.0, 0.0, 0.0), Vec3d(1.0, 0.0, 0.0),
        Vec3d::Zero(), Vec3d(0.0, 1.0, 0.0),
        iPt));
}

TEST(imstkCollisionUtilsTest, RayToSphere)
{
    // \todo: RayToSphere does not report exit point

    // Test ray outside of sphere miss and hit
    Vec3d iPt;
    EXPECT_TRUE(CollisionUtils::testRayToSphere(
        Vec3d(1.0, 0.0, 0.0), Vec3d(-1.0, 0.0, 0.0),
        Vec3d::Zero(), 0.5,
        iPt));
    EXPECT_FALSE(CollisionUtils::testRayToSphere(
        Vec3d(1.0, 1.0, 0.0), Vec3d(-1.0, 0.0, 0.0),
        Vec3d::Zero(), 0.5,
        iPt));

    // Test ray inside the sphere hit
    EXPECT_TRUE(CollisionUtils::testRayToSphere(
        Vec3d(0.0, 0.0, 0.0), Vec3d(1.0, 0.0, 0.0),
        Vec3d::Zero(), 0.5,
        iPt));
}

TEST(imstkColliosUtilsTest, PlaneToSphere)
{
    const double eps    = 1e-10;
    Vec3d        planeP = Vec3d(0, 0, 0);
    Vec3d        planeN = Vec3d(0, 1, 0);

    Vec3d  sphereP = Vec3d(0, 3, 0);
    double sphereR = 2.0;

    Vec3d  planeContactP, planeContactN;
    Vec3d  sphereContactP, sphereContactN;
    double depth;

    // Outside
    EXPECT_FALSE(testPlaneToSphere(planeP, planeN, sphereP, sphereR,
        planeContactP, planeContactN, sphereContactP, sphereContactN, depth));
    EXPECT_FALSE(testPlaneToSphere(planeP, planeN, sphereP, sphereR));

    // Glancing
    sphereP = Vec3d(0, 2, 0);

    EXPECT_FALSE(testPlaneToSphere(planeP, planeN, sphereP, sphereR,
        planeContactP, planeContactN, sphereContactP, sphereContactN, depth));
    EXPECT_FALSE(testPlaneToSphere(planeP, planeN, sphereP, sphereR));

    // A little inside
    sphereP = Vec3d(0, 1, 0);

    EXPECT_TRUE(testPlaneToSphere(planeP, planeN, sphereP, sphereR,
        planeContactP, planeContactN, sphereContactP, sphereContactN, depth));
    EXPECT_TRUE(testPlaneToSphere(planeP, planeN, sphereP, sphereR));
    EXPECT_DOUBLE_EQ(depth, 1);
    EXPECT_TRUE(sphereContactP.isApprox(Vec3d(0, -1, 0)));
    EXPECT_TRUE(planeContactP.isApprox(Vec3d(0, 0, 0)));

    // Edge Case
    sphereP = Vec3d(0, 0, 0);
    EXPECT_TRUE(testPlaneToSphere(planeP, planeN, sphereP, sphereR,
        planeContactP, planeContactN, sphereContactP, sphereContactN, depth));
    EXPECT_TRUE(testPlaneToSphere(planeP, planeN, sphereP, sphereR));
    EXPECT_DOUBLE_EQ(depth, 2);
    EXPECT_TRUE(sphereContactP.isApprox(Vec3d(0, -2, 0)));
    EXPECT_TRUE(planeContactP.isApprox(Vec3d(0, 0, 0)));
}
} // namespace CollisionUtils
} // namespace imstk
