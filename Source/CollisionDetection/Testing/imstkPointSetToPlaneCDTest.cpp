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

#include "imstkPlane.h"
#include "imstkPointSet.h"
#include "imstkPointSetToPlaneCD.h"
#include "imstkVecDataArray.h"

#include <gtest/gtest.h>

using namespace imstk;

TEST(imstkPointSetToPlaneCDTest, IntersectionTestAB)
{
    auto plane = std::make_shared<Plane>();

    auto pointSet    = std::make_shared<PointSet>();
    auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(1);
    (*verticesPtr)[0] = Vec3d(0.0, -0.5, 0.0);
    pointSet->initialize(verticesPtr);

    PointSetToPlaneCD m_pointSetToPlaneCD;
    m_pointSetToPlaneCD.setInput(pointSet, 0);
    m_pointSetToPlaneCD.setInput(plane, 1);
    m_pointSetToPlaneCD.setGenerateCD(true, true);
    m_pointSetToPlaneCD.update();

    std::shared_ptr<CollisionData> colData = m_pointSetToPlaneCD.getCollisionData();

    // Should be one element on side A, 0 on side B (default CD data is not generated for the sphere)
    EXPECT_EQ(1, colData->elementsA.size());
    EXPECT_EQ(1, colData->elementsB.size());

    // That element should be a point directional element
    EXPECT_EQ(CollisionElementType::PointIndexDirection, colData->elementsA[0].m_type);
    EXPECT_EQ(CollisionElementType::PointDirection, colData->elementsB[0].m_type);

    // That element should have 0.5 depth
    EXPECT_EQ(Vec3d(0.0, 1.0, 0.0), colData->elementsA[0].m_element.m_PointIndexDirectionElement.dir);
    EXPECT_EQ(Vec3d(0.0, -1.0, 0.0), colData->elementsB[0].m_element.m_PointDirectionElement.dir);

    // Should have depths of 0.5
    EXPECT_NEAR(0.5, colData->elementsA[0].m_element.m_PointIndexDirectionElement.penetrationDepth, 1.0e-4);
    EXPECT_NEAR(0.5, colData->elementsB[0].m_element.m_PointDirectionElement.penetrationDepth, 1.0e-4);

    // The contact point on A should be the point
    EXPECT_EQ(0, colData->elementsA[0].m_element.m_PointIndexDirectionElement.ptIndex);

    // The contact point on B should be the nearest point on the surface of the sphere
    EXPECT_NEAR(-0.5, colData->elementsB[0].m_element.m_PointDirectionElement.pt[1], 1.0e-4);
}

TEST(imstkPointSetToPlaneCDTest, NonIntersectionTestAB)
{
    auto plane       = std::make_shared<Plane>(Vec3d(0.0, 0.0, 0.0), Vec3d(1., 1., 1.));
    auto pointSet    = std::make_shared<PointSet>();
    auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(1);
    (*verticesPtr)[0] = Vec3d(1.0, 1.0, 1.0);
    pointSet->initialize(verticesPtr);

    PointSetToPlaneCD m_pointSetToPlaneCD;
    m_pointSetToPlaneCD.setInput(pointSet, 0);
    m_pointSetToPlaneCD.setInput(plane, 1);
    m_pointSetToPlaneCD.setGenerateCD(true, true);
    m_pointSetToPlaneCD.update();

    std::shared_ptr<CollisionData> colData = m_pointSetToPlaneCD.getCollisionData();

    // Should be no elements
    EXPECT_EQ(0, colData->elementsA.size());
    EXPECT_EQ(0, colData->elementsB.size());
}