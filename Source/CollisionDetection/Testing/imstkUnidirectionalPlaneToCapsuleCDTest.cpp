/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "gtest/gtest.h"

#include "imstkUnidirectionalPlaneToCapsuleCD.h"
#include "imstkCapsule.h"
#include "imstkPlane.h"

using namespace imstk;

TEST(imstkUnidirectionalPlaneToCapsuleCDTest, IntersectionTestAB)
{
    auto capsule = std::make_shared<Capsule>(Vec3d(0.0, 0.0, 0.0), 0.1, 1.0);
    auto plane   = std::make_shared<Plane>(Vec3d(0.0, 0.0, 0.0), Vec3d(0.0, 1.0, 0.0));

    UnidirectionalPlaneToCapsuleCD planeToCapsuleCD;
    planeToCapsuleCD.setInput(capsule, 0);
    planeToCapsuleCD.setInput(plane, 1);
    planeToCapsuleCD.setGenerateCD(true, true); // Generate both A and B
    planeToCapsuleCD.update();

    std::shared_ptr<CollisionData> colData = planeToCapsuleCD.getCollisionData();

    // Should be one element per side
    EXPECT_EQ(1, colData->elementsA.size());
    EXPECT_EQ(1, colData->elementsB.size());

    // That element should be a point directional element
    EXPECT_EQ(CollisionElementType::PointDirection, colData->elementsA[0].m_type);
    EXPECT_EQ(CollisionElementType::PointDirection, colData->elementsB[0].m_type);

    // Direction to resolve/push
    EXPECT_EQ(Vec3d(0.0, 1.0, 0.0), colData->elementsA[0].m_element.m_PointDirectionElement.dir);
    EXPECT_EQ(Vec3d(0.0, -1.0, 0.0), colData->elementsB[0].m_element.m_PointDirectionElement.dir);

    // Amount to resolve
    EXPECT_EQ(0.6, colData->elementsA[0].m_element.m_PointDirectionElement.penetrationDepth);
    EXPECT_EQ(0.6, colData->elementsB[0].m_element.m_PointDirectionElement.penetrationDepth);

    // Contact point on shape
    EXPECT_EQ(Vec3d(0.0, -0.6, 0.0), colData->elementsA[0].m_element.m_PointDirectionElement.pt);
    EXPECT_EQ(Vec3d(0.0, 0.0, 0.0), colData->elementsB[0].m_element.m_PointDirectionElement.pt);
}

TEST(imstkUnidirectionalPlaneToCapsuleCDTest, NonIntersectionTestAB)
{
    auto capsule = std::make_shared<Capsule>(Vec3d(0.0, 2.0, 0.0), 0.1, 1.0);
    auto plane   = std::make_shared<Plane>(Vec3d(0.0, 0.0, 0.0), Vec3d(0.0, 1.0, 0.0));

    UnidirectionalPlaneToCapsuleCD planeToCapsuleCD;
    planeToCapsuleCD.setInput(capsule, 0);
    planeToCapsuleCD.setInput(plane, 1);
    planeToCapsuleCD.setGenerateCD(true, true); // Generate both A and B
    planeToCapsuleCD.update();

    std::shared_ptr<CollisionData> colData = planeToCapsuleCD.getCollisionData();
    EXPECT_EQ(0, colData->elementsA.size());
    EXPECT_EQ(0, colData->elementsB.size());
}