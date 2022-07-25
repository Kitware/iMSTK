/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkOrientedBox.h"
#include "imstkPointSet.h"
#include "imstkPointSetToOrientedBoxCD.h"
#include "imstkVecDataArray.h"

#include <gtest/gtest.h>

using namespace imstk;

TEST(imstkPointSetToOrientedBoxCDTest, IntersectionTestAB)
{
    // Cube with width 2 (max y is 1)
    auto cube        = std::make_shared<OrientedBox>(Vec3d(0.0, 0.0, 0.0), Vec3d(1.0, 1.0, 1.0));
    auto pointSet    = std::make_shared<PointSet>();
    auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(1);
    (*verticesPtr)[0] = Vec3d(0.0, 0.9, 0.0);
    pointSet->initialize(verticesPtr);

    PointSetToOrientedBoxCD m_pointSetToOrientedBoxCD;
    m_pointSetToOrientedBoxCD.setInput(pointSet, 0);
    m_pointSetToOrientedBoxCD.setInput(cube, 1);
    m_pointSetToOrientedBoxCD.setGenerateCD(true, true); // Generate both A and B
    m_pointSetToOrientedBoxCD.update();

    std::shared_ptr<CollisionData> colData = m_pointSetToOrientedBoxCD.getCollisionData();

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
    EXPECT_NEAR(0.1, colData->elementsA[0].m_element.m_PointIndexDirectionElement.penetrationDepth, 0.00001);
    EXPECT_NEAR(0.1, colData->elementsB[0].m_element.m_PointDirectionElement.penetrationDepth, 0.00001);

    // The contact point on A should be the point
    EXPECT_EQ(0, colData->elementsA[0].m_element.m_PointIndexDirectionElement.ptIndex);
    // The contact point on B should be the nearest point on the surface of the cube
    EXPECT_NEAR(1.0, colData->elementsB[0].m_element.m_PointDirectionElement.pt[1], 0.00001);
}

TEST(imstkPointSetToOrientedBoxCDTest, NonIntersectionTestAB)
{
    // Cube with width 2 (max y is 1)
    auto cube        = std::make_shared<OrientedBox>(Vec3d(0.0, 0.0, 0.0), Vec3d(1.0, 1.0, 1.0));
    auto pointSet    = std::make_shared<PointSet>();
    auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(1);
    (*verticesPtr)[0] = Vec3d(0.0, 1.1, 0.0);
    pointSet->initialize(verticesPtr);

    PointSetToOrientedBoxCD m_pointSetToOrientedBoxCD;
    m_pointSetToOrientedBoxCD.setInput(pointSet, 0);
    m_pointSetToOrientedBoxCD.setInput(cube, 1);
    m_pointSetToOrientedBoxCD.setGenerateCD(true, true); // Generate both A and B
    m_pointSetToOrientedBoxCD.update();

    std::shared_ptr<CollisionData> colData = m_pointSetToOrientedBoxCD.getCollisionData();

    // Should be one element on side A, 0 on side B (default CD data is not generated for the sphere)
    EXPECT_EQ(0, colData->elementsA.size());
    EXPECT_EQ(0, colData->elementsB.size());
}