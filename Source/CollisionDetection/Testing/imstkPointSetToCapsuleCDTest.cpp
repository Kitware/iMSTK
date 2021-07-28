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

#include "imstkPointSet.h"
#include "imstkPointSetToCapsuleCD.h"
#include "imstkCapsule.h"

using namespace imstk;

TEST(imstkPointSetToCapsuleCDTest, IntersectionTestAB)
{
    PointSetToCapsuleCD m_pointSetToCapsuleCD;

    auto capsule = std::make_shared<Capsule>();

    auto pointSet    = std::make_shared<PointSet>();
    auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(1);
    (*verticesPtr)[0] = Vec3d(0.25, 0.0, 0.0);
    pointSet->initialize(verticesPtr);

    m_pointSetToCapsuleCD.setInput(pointSet, 0);
    m_pointSetToCapsuleCD.setInput(capsule, 1);
    m_pointSetToCapsuleCD.setGenerateCD(true, true);
    m_pointSetToCapsuleCD.update();

    std::shared_ptr<CollisionData> colData = m_pointSetToCapsuleCD.getCollisionData();

    // Should be one element on side A, 0 on side B (default CD data is not generated for the sphere)
    EXPECT_EQ(1, colData->elementsA.getSize());
    EXPECT_EQ(1, colData->elementsB.getSize());

    // That element should be a point directional element
    EXPECT_EQ(CollisionElementType::PointIndexDirection, colData->elementsA[0].m_type);
    EXPECT_EQ(CollisionElementType::PointDirection, colData->elementsB[0].m_type);

    EXPECT_EQ(Vec3d(1.0, 0.0, 0.0), colData->elementsA[0].m_element.m_PointIndexDirectionElement.dir);
    EXPECT_EQ(Vec3d(-1.0, 0.0, 0.0), colData->elementsB[0].m_element.m_PointDirectionElement.dir);

    // Should have depth
    EXPECT_NEAR(0.25, colData->elementsA[0].m_element.m_PointIndexDirectionElement.penetrationDepth, 1.0e-4);
    EXPECT_NEAR(0.25, colData->elementsB[0].m_element.m_PointDirectionElement.penetrationDepth, 1.0e-4);

    // The contact point on A should be the point
    EXPECT_EQ(0, colData->elementsA[0].m_element.m_PointIndexDirectionElement.ptIndex);

    // The contact point on B should be the nearest point on the surface of the sphere
    EXPECT_NEAR(0.5, colData->elementsB[0].m_element.m_PointDirectionElement.pt[0], 1.0e-4);
}

TEST(imstkPointSetToCapsuleCDTest, NonIntersectionTestAB)
{
    PointSetToCapsuleCD m_pointSetToCapsuleCD;

    auto capsule     = std::make_shared<Capsule>();
    auto pointSet    = std::make_shared<PointSet>();
    auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(1);
    (*verticesPtr)[0] = Vec3d(5.0, 5.0, 5.0);
    pointSet->initialize(verticesPtr);

    m_pointSetToCapsuleCD.setInput(pointSet, 0);
    m_pointSetToCapsuleCD.setInput(capsule, 1);
    m_pointSetToCapsuleCD.setGenerateCD(true, true);
    m_pointSetToCapsuleCD.update();

    std::shared_ptr<CollisionData> colData = m_pointSetToCapsuleCD.getCollisionData();

    // Should be no elements
    EXPECT_EQ(0, colData->elementsA.getSize());
    EXPECT_EQ(0, colData->elementsB.getSize());
}

int
imstkPointSetToCapsuleCDTest(int argc, char* argv[])
{
    // Init Google Test & Mock
    ::testing::InitGoogleTest(&argc, argv);

    // Run tests with gtest
    return RUN_ALL_TESTS();
}
