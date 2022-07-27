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

#include "imstkCapsule.h"
#include "imstkCDObjectFactory.h"
#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkLineMesh.h"
#include "imstkPointSet.h"
#include "imstkPointSetToCapsuleCD.h"

using namespace imstk;

///
/// \brief makeCollisionDetection tests the makeCollisionDetection function in the CDObjectFactory.
/// Currently there is a test to verify a known cdType, and a death test on an invalid type
TEST(imsktCDObjectFactoryTest, makeCollisionDetection)
{
    std::string cdType = PointSetToCapsuleCD::getStaticTypeName();

    // Setup the CD
    std::shared_ptr<CollisionDetectionAlgorithm> cd = CDObjectFactory::makeCollisionDetection(cdType);
    std::string                                  returnedType = cd->getTypeName();

    // Verify that the CDObjectFactory generates the correct collision detection type
    EXPECT_EQ(cdType, returnedType);

    // Verify death on invalid cdType
    std::string cdFail = "PeanutButterCD";
    EXPECT_DEATH(CDObjectFactory::makeCollisionDetection(cdFail), "No collision detection type named: PeanutButterCD");
}

TEST(imsktCDObjectFactoryTest, getCDType)
{
    // Verify known case for CDType, forwards and backwards
    auto capsule  = std::make_shared<Capsule>();
    auto pointSet = std::make_shared<PointSet>();

    auto cdTypeTest1 = CDObjectFactory::getCDType(*capsule, *pointSet);
    EXPECT_EQ(cdTypeTest1, std::string("PointSetToCapsuleCD"));

    auto cdTypeTest2 = CDObjectFactory::getCDType(*pointSet, *capsule);
    EXPECT_EQ(cdTypeTest2, std::string("PointSetToCapsuleCD"));

    //Verify invalid case for CDType
    auto lineMesh = std::make_shared<LineMesh>();
    EXPECT_DEATH(CDObjectFactory::getCDType(*lineMesh, *pointSet), "No valid collision detection type for : LineMeshPointSet");
}