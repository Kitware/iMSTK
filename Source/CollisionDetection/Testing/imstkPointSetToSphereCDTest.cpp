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

#include <tuple>

#include "gtest/gtest.h"

#include "imstkPointSet.h"
#include "imstkPointSetToSphereCD.h"
#include "imstkSphere.h"

using namespace imstk;

struct pointSetSphereCDTestData
{
    Vec3d spherePos;     // sphere position
    double sphereRadius; // sphere position

    Vec3d point;         // point position to be tested against sphere
};

std::ostream&
operator<<(std::ostream& stream, pointSetSphereCDTestData const& d)
{
    return stream << "[pointSetSphereCDTestData: Sphere (pos=" << d.spherePos.transpose()
                  << " rad=" << d.sphereRadius << "), Point (pos=" << d.point.transpose() << "]";
}

struct pointSetSphereCDValidationData
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
operator<<(std::ostream& stream, pointSetSphereCDValidationData const& v)
{
    return stream << "[pointSetCapsuleCDValidationData: " << v.sizeA << ", "
                  << v.sizeB << ", "
                  << static_cast<int>(v.elementTypeA) << ", "
                  << static_cast<int>(v.elementTypeB) << ", ("
                  << v.dirA.transpose() << "), ("
                  << v.dirB.transpose() << "), "
                  << v.depthA << ", "
                  << v.depthB << ", "
                  << v.contactIndexA << ", ("
                  << v.contactPointB.transpose() << ") ]";
}

class imstkPointSetToSphereCDIntersectionTests :
    public ::testing::TestWithParam<std::tuple<pointSetSphereCDTestData, pointSetSphereCDValidationData>> {};

TEST_P(imstkPointSetToSphereCDIntersectionTests, IntersectionTestAB)
{
    const pointSetSphereCDTestData&       testData = std::get<0>(GetParam());
    const pointSetSphereCDValidationData& valData  = std::get<1>(GetParam());

    auto sphere      = std::make_shared<Sphere>(testData.spherePos, testData.sphereRadius);
    auto pointSet    = std::make_shared<PointSet>();
    auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(1);
    (*verticesPtr)[0] = testData.point;
    pointSet->initialize(verticesPtr);

    PointSetToSphereCD m_pointSetToSphereCD;
    m_pointSetToSphereCD.setInput(pointSet, 0);
    m_pointSetToSphereCD.setInput(sphere, 1);
    m_pointSetToSphereCD.setGenerateCD(true, true); // Generate both A and B
    m_pointSetToSphereCD.update();

    std::shared_ptr<CollisionData> colData = m_pointSetToSphereCD.getCollisionData();

    // Should be one element on side A, 0 on side B (default CD data is not generated for the sphere)
    EXPECT_EQ(valData.sizeA, colData->elementsA.size());
    EXPECT_EQ(valData.sizeB, colData->elementsB.size());

    // That element should be a point directional element
    EXPECT_EQ(valData.elementTypeA, colData->elementsA[0].m_type);
    EXPECT_EQ(valData.elementTypeB, colData->elementsB[0].m_type);

    // That element should have 0.5 depth
    EXPECT_EQ(valData.dirA, colData->elementsA[0].m_element.m_PointIndexDirectionElement.dir);
    EXPECT_EQ(valData.dirB, colData->elementsB[0].m_element.m_PointDirectionElement.dir);

    // Should have depths of 0.5
    EXPECT_NEAR(valData.depthA, colData->elementsA[0].m_element.m_PointIndexDirectionElement.penetrationDepth, 0.00001);
    EXPECT_NEAR(valData.depthB, colData->elementsB[0].m_element.m_PointDirectionElement.penetrationDepth, 0.00001);

    // The contact point on A should be the point
    EXPECT_EQ(valData.contactIndexA, colData->elementsA[0].m_element.m_PointIndexDirectionElement.ptIndex);

    // The contact point on B should be the nearest point on the surface of the sphere
    EXPECT_LE((valData.contactPointB - colData->elementsB[0].m_element.m_PointDirectionElement.pt).norm(), 1.0e-12);
}

INSTANTIATE_TEST_SUITE_P(PointSetToSphereCDTests, imstkPointSetToSphereCDIntersectionTests,
    ::testing::Values(std::make_tuple(pointSetSphereCDTestData{ Vec3d(0.0, -2.0, 0.0), 2.1, Vec3d(0.0, 0.0, 0.0) },
                                      pointSetSphereCDValidationData{ 1, 1, CollisionElementType::PointIndexDirection,
                                                                      CollisionElementType::PointDirection,
                                                                      Vec3d(0.0, 1.0, 0.0), Vec3d(0.0, -1.0, 0.0),
                                                                      0.1, 0.1,
                                                                      0, Vec3d(0.0, 0.1, 0.0) })));

class imstkPointSetToSphereCDNonIntersectionTests : public ::testing::TestWithParam<pointSetSphereCDTestData> {};

TEST_P(imstkPointSetToSphereCDNonIntersectionTests, NonIntersectionTestAB)
{
    const pointSetSphereCDTestData testData = GetParam();

    auto sphere      = std::make_shared<Sphere>(testData.spherePos, testData.sphereRadius);
    auto pointSet    = std::make_shared<PointSet>();
    auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(1);
    (*verticesPtr)[0] = testData.point;
    pointSet->initialize(verticesPtr);

    PointSetToSphereCD m_pointSetToSphereCD;
    m_pointSetToSphereCD.setInput(pointSet, 0);
    m_pointSetToSphereCD.setInput(sphere, 1);
    m_pointSetToSphereCD.setGenerateCD(true, true); // Generate both A and B
    m_pointSetToSphereCD.update();

    std::shared_ptr<CollisionData> colData = m_pointSetToSphereCD.getCollisionData();

    // Should be no elements
    EXPECT_EQ(0, colData->elementsA.size());
    EXPECT_EQ(0, colData->elementsB.size());
}

INSTANTIATE_TEST_SUITE_P(PointSetToSphereCDTests, imstkPointSetToSphereCDNonIntersectionTests,
    ::testing::Values(pointSetSphereCDTestData{ Vec3d(0.0, -8.0, 0.0), 2.1, Vec3d(0.0, 0.0, 0.0) },
                      pointSetSphereCDTestData{ Vec3d(0.0, 0.0, 0.0), 1.0, Vec3d(0.0, 5.0, 0.0) },
                      pointSetSphereCDTestData{ Vec3d(0.0, 0.0, 0.0), 0.1, Vec3d(2.0, 0.0, 0.0) },
                      pointSetSphereCDTestData{ Vec3d(0.0, 0.0, 7.4), 5.0, Vec3d(0.0, 0.0, -10.0) }));
