/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "gtest/gtest.h"

#include "imstkSphereToCapsuleCD.h"
#include "imstkCapsule.h"
#include "imstkSphere.h"

using namespace imstk;

TEST(imstkSphereToCapsuleCDTest, IntersectionTestAB)
{
    auto sphere  = std::make_shared<Sphere>(Vec3d(0.15, 0.0, 0.0), 0.1);
    auto capsule = std::make_shared<Capsule>(Vec3d(0.0, 0.0, 0.0), 0.1, 1.0);

    SphereToCapsuleCD sphereToCapsuleCD;
    sphereToCapsuleCD.setInput(sphere, 0);
    sphereToCapsuleCD.setInput(capsule, 1);
    sphereToCapsuleCD.setGenerateCD(true, true); // Generate both A and B
    sphereToCapsuleCD.update();

    std::shared_ptr<CollisionData> colData = sphereToCapsuleCD.getCollisionData();

    // Should be one element per side
    EXPECT_EQ(1, colData->elementsA.size());
    EXPECT_EQ(1, colData->elementsB.size());

    // That element should be a point directional element
    EXPECT_EQ(CollisionElementType::PointDirection, colData->elementsA[0].m_type);
    EXPECT_EQ(CollisionElementType::PointDirection, colData->elementsB[0].m_type);

    // Direction to resolve/push
    EXPECT_EQ(Vec3d(1.0, 0.0, 0.0), colData->elementsA[0].m_element.m_PointDirectionElement.dir);
    EXPECT_EQ(Vec3d(-1.0, 0.0, 0.0), colData->elementsB[0].m_element.m_PointDirectionElement.dir);

    // Amount to resolve
    EXPECT_NEAR(0.05, colData->elementsA[0].m_element.m_PointDirectionElement.penetrationDepth, 0.000001);
    EXPECT_NEAR(0.05, colData->elementsB[0].m_element.m_PointDirectionElement.penetrationDepth, 0.000001);

    // Contact point on shape
    EXPECT_TRUE(colData->elementsA[0].m_element.m_PointDirectionElement.pt.isApprox(Vec3d(0.05, 0.0, 0.0), 0.000001));
    EXPECT_TRUE(colData->elementsB[0].m_element.m_PointDirectionElement.pt.isApprox(Vec3d(0.1, 0.0, 0.0), 0.000001));
}

TEST(imstkSphereToCapsuleCDTest, NonIntersectionTestAB)
{
    auto capsule = std::make_shared<Capsule>(Vec3d(0.0, 0.0, 0.0), 0.1, 1.0);
    auto sphere  = std::make_shared<Sphere>(Vec3d(1.15, 0.0, 0.0), 0.1);

    SphereToCapsuleCD sphereToCapsuleCD;
    sphereToCapsuleCD.setInput(sphere, 0);
    sphereToCapsuleCD.setInput(capsule, 1);
    sphereToCapsuleCD.setGenerateCD(true, true); // Generate both A and B
    sphereToCapsuleCD.update();

    std::shared_ptr<CollisionData> colData = sphereToCapsuleCD.getCollisionData();

    // Should be one element per side
    EXPECT_EQ(0, colData->elementsA.size());
    EXPECT_EQ(0, colData->elementsB.size());
}