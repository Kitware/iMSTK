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

#include "imstkCapsule.h"

#include <gtest/gtest.h>

using namespace imstk;

TEST(imstkCapsuleTest, SetGetRadius)
{

   Capsule capsule;

   capsule.setRadius(2);
   EXPECT_DOUBLE_EQ(2, capsule.getRadius());

   capsule.setRadius(9000000);
   EXPECT_DOUBLE_EQ(9000000, capsule.getRadius());


   capsule.setRadius(0.0000002);
   EXPECT_DOUBLE_EQ(0.0000002, capsule.getRadius());


   EXPECT_DEATH(capsule.setRadius(-50),"Capsule::setRadius error: radius should be positive.");

}

TEST(imstkCapsuleTest, SetGetLength)
{

   Capsule capsule;

   capsule.setLength(10);
   EXPECT_DOUBLE_EQ(10, capsule.getLength());

   capsule.setLength(0.000004);
   EXPECT_DOUBLE_EQ(0.000004, capsule.getLength());

   capsule.setLength(1000000000);
   EXPECT_DOUBLE_EQ(1000000000, capsule.getLength());

   EXPECT_DEATH(capsule.setLength(0), "Capsule::setLength error: length should be positive.");
   EXPECT_DEATH(capsule.setLength(-10), "Capsule::setLength error: length should be positive.");

}

TEST(imstkCapsuleTest, GetVolume)
{

   Capsule capsule; 

   capsule.setLength(2.0);
   capsule.setRadius(1.0);
   EXPECT_DOUBLE_EQ(
      PI * capsule.getRadius() * capsule.getRadius() 
      * (capsule.getLength() + 4.0 / 3.0 * capsule.getRadius()), 
      capsule.getVolume());

   capsule.setLength(20000.0);
   capsule.setRadius(10000000.0);
   EXPECT_DOUBLE_EQ(
      PI * capsule.getRadius() * capsule.getRadius() 
      * (capsule.getLength() + 4.0 / 3.0 * capsule.getRadius()), 
      capsule.getVolume());

}

TEST(imstkCapsuleTest, GetFunctionalValue)
{

   Capsule capsule; 

   capsule.setLength(2.0);
   capsule.setRadius(1.0);
   capsule.updatePostTransformData();

   EXPECT_DOUBLE_EQ(0.0, capsule.getFunctionValue(Vec3d(1.0, 0.0, 0.0)));
   EXPECT_DOUBLE_EQ(0.0, capsule.getFunctionValue(Vec3d(0.0, 0.0, 1.0)));
   EXPECT_DOUBLE_EQ(0.0, capsule.getFunctionValue(Vec3d(0.0, 2.0, 0.0)));

   EXPECT_DOUBLE_EQ(0.0, capsule.getFunctionValue(Vec3d(-1.0, 0.0, 0.0)));
   EXPECT_DOUBLE_EQ(0.0, capsule.getFunctionValue(Vec3d(0.0, 0.0, -1.0)));
   EXPECT_DOUBLE_EQ(0.0, capsule.getFunctionValue(Vec3d(0.0, -2.0, 0.0)));
    
   EXPECT_DOUBLE_EQ(1.0, capsule.getFunctionValue(Vec3d(2.0, 0.0, 0.0)));
   EXPECT_DOUBLE_EQ(1.0, capsule.getFunctionValue(Vec3d(0.0, 0.0, 2.0)));
   EXPECT_DOUBLE_EQ(1.0, capsule.getFunctionValue(Vec3d(0.0, 3.0, 0.0)));

   EXPECT_DOUBLE_EQ(1.0, capsule.getFunctionValue(Vec3d(-2.0, 0.0, 0.0)));
   EXPECT_DOUBLE_EQ(1.0, capsule.getFunctionValue(Vec3d(0.0, 0.0, -2.0)));
   EXPECT_DOUBLE_EQ(1.0, capsule.getFunctionValue(Vec3d(0.0, -3.0, 0.0)));


   EXPECT_DOUBLE_EQ(-0.5, capsule.getFunctionValue(Vec3d(0.5, 0.0, 0.0)));
   EXPECT_DOUBLE_EQ(-0.5, capsule.getFunctionValue(Vec3d(0.0, 0.0, 0.5)));
   EXPECT_DOUBLE_EQ(-1.0, capsule.getFunctionValue(Vec3d(0.0, 0.0, 0.0)));

}