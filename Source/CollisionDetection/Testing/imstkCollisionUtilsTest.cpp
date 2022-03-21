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

using namespace imstk;

TEST(imstkCollisionUtilsTest, RayToPlaneTest)
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