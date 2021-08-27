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
#include <iostream>

using namespace imstk;

struct pointSetCapsuleCDTestData
{
    // Capsule parameters
    Vec3d capsulePos;
    double capsuleRadius;
    double capsuleLength;

    Vec3d point;// point position to be tested against capsule
};

std::ostream&
operator<<(std::ostream& stream, pointSetCapsuleCDTestData const& d)
{
    return stream << "[pointSetCapsuleCDTestData: Capsule (pos=" << d.capsulePos.transpose()
                  << " rad=" << d.capsuleRadius << " len=" << d.capsuleLength << "), Point (pos=" << d.capsulePos.transpose() << "]";
}

struct pointSetCapsuleCDValidationData
{
    // size of collision data
    size_t sizeA;
    size_t sizeB;

    // collision element type on both sides
    CollisionElementType elementTypeA;
    CollisionElementType elementTypeB;

    // contact directions
    Vec3d dirA;
    Vec3d dirB;

    // contact depths
    double depthA;
    double depthB;

    size_t contactIndexA;              // index of the contact point side A
    Vec3d contactPointB;               // position of the contact point side B
};

std::ostream&
operator<<(std::ostream& stream, pointSetCapsuleCDValidationData const& v)
{
    return stream << "[pointSetCapsuleCDValidationData: " << v.sizeA << ", "
                  << v.sizeB << ", "
                  << (int)v.elementTypeA << ", "
                  << (int)v.elementTypeB << ", ("
                  << v.dirA.transpose() << "), ("
                  << v.dirB.transpose() << "), "
                  << v.depthA << ", "
                  << v.depthB << ", "
                  << v.contactIndexA << ", ("
                  << v.contactPointB.transpose() << ") ]";
}

class imstkPointSetToCapsuleCDIntersectionTests :
    public ::testing::TestWithParam<std::tuple<pointSetCapsuleCDTestData, pointSetCapsuleCDValidationData>> {};

TEST_P(imstkPointSetToCapsuleCDIntersectionTests, IntersectionTestAB)
{
    const pointSetCapsuleCDTestData&       testData = std::get<0>(GetParam());
    const pointSetCapsuleCDValidationData& valData  = std::get<1>(GetParam());

    auto capsule = std::make_shared<Capsule>(testData.capsulePos, testData.capsuleRadius, testData.capsuleLength);

    auto pointSet    = std::make_shared<PointSet>();
    auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(1);
    (*verticesPtr)[0] = testData.point;
    pointSet->initialize(verticesPtr);

    PointSetToCapsuleCD m_pointSetToCapsuleCD;
    m_pointSetToCapsuleCD.setInput(pointSet, 0);
    m_pointSetToCapsuleCD.setInput(capsule, 1);
    m_pointSetToCapsuleCD.setGenerateCD(true, true);
    m_pointSetToCapsuleCD.update();

    std::shared_ptr<CollisionData> colData = m_pointSetToCapsuleCD.getCollisionData();

    // Should be one element on side A, 0 on side B (default CD data is not generated for the sphere)
    EXPECT_EQ(valData.sizeA, colData->elementsA.size());
    EXPECT_EQ(valData.sizeB, colData->elementsB.size());

    // That element should be a point directional element
    EXPECT_EQ(valData.elementTypeA, colData->elementsA[0].m_type);
    EXPECT_EQ(valData.elementTypeB, colData->elementsB[0].m_type);

    EXPECT_EQ(valData.dirA, colData->elementsA[0].m_element.m_PointIndexDirectionElement.dir);
    EXPECT_EQ(valData.dirB, colData->elementsB[0].m_element.m_PointDirectionElement.dir);

    // Should have depth
    EXPECT_NEAR(valData.depthA, colData->elementsA[0].m_element.m_PointIndexDirectionElement.penetrationDepth, 1.0e-4);
    EXPECT_NEAR(valData.depthB, colData->elementsB[0].m_element.m_PointDirectionElement.penetrationDepth, 1.0e-4);

    // The contact point on A should be the point
    EXPECT_EQ(valData.contactIndexA, colData->elementsA[0].m_element.m_PointIndexDirectionElement.ptIndex);

    // The contact point on B should be the nearest point on the surface of the sphere
    EXPECT_LE((valData.contactPointB - colData->elementsB[0].m_element.m_PointDirectionElement.pt).norm(), 1.0e-12);
}

INSTANTIATE_TEST_SUITE_P(PointSetToCapsuleCDTests, imstkPointSetToCapsuleCDIntersectionTests,
    ::testing::Values(std::make_tuple(pointSetCapsuleCDTestData{ Vec3d(0.0, 0.0, 0.0), 0.5, 1.0, Vec3d(0.25, 0.0, 0.0) },
        pointSetCapsuleCDValidationData{ 1, 1, CollisionElementType::PointIndexDirection,
                                         CollisionElementType::PointDirection,
                                         Vec3d(1.0, 0.0, 0.0), Vec3d(-1.0, 0.0, 0.0),
                                         0.25, 0.25,
                                         0, Vec3d(0.5, 0.0, 0.0) })
        ));

class imstkPointSetToCapsuleCDNonIntersectionTests : public ::testing::TestWithParam<pointSetCapsuleCDTestData> {};

TEST_P(imstkPointSetToCapsuleCDNonIntersectionTests, NonIntersectionTestAB)
{
    const pointSetCapsuleCDTestData testData = GetParam();

    auto capsule     = std::make_shared<Capsule>(testData.capsulePos, testData.capsuleRadius, testData.capsuleLength);
    auto pointSet    = std::make_shared<PointSet>();
    auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(1);
    (*verticesPtr)[0] = testData.point;
    pointSet->initialize(verticesPtr);

    PointSetToCapsuleCD m_pointSetToCapsuleCD;
    m_pointSetToCapsuleCD.setInput(pointSet, 0);
    m_pointSetToCapsuleCD.setInput(capsule, 1);
    m_pointSetToCapsuleCD.setGenerateCD(true, true);
    m_pointSetToCapsuleCD.update();

    std::shared_ptr<CollisionData> colData = m_pointSetToCapsuleCD.getCollisionData();

    // check if there are no contacts
    EXPECT_EQ(0, colData->elementsA.size());
    EXPECT_EQ(0, colData->elementsB.size());
}

INSTANTIATE_TEST_SUITE_P(PointSetToCapsuleCDTests, imstkPointSetToCapsuleCDNonIntersectionTests,
    ::testing::Values(pointSetCapsuleCDTestData{ Vec3d(0.0, 0.0, 0.0), 0.5, 1.0, Vec3d(5.0, 5.0, 5.0) }));