/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "gtest/gtest.h"

#include "imstkSphereToCylinderCD.h"
#include "imstkSphere.h"
#include "imstkCylinder.h"

using namespace imstk;

TEST(imstkSphereToCylinderCDTest, IntersectionTestAB)
{
    auto sphere   = std::make_shared<Sphere>(Vec3d(0.0, 1.0, 0.0), 0.75);
    auto cylinder = std::make_shared<Cylinder>();

    SphereToCylinderCD m_sphereToCylinderCD;
    m_sphereToCylinderCD.setInput(sphere, 0);
    m_sphereToCylinderCD.setInput(cylinder, 1);
    m_sphereToCylinderCD.setGenerateCD(true, true); // Generate both A and B
    m_sphereToCylinderCD.update();

    std::shared_ptr<CollisionData> colData = m_sphereToCylinderCD.getCollisionData();

    // Should be one element per side
    EXPECT_EQ(1, colData->elementsA.size());
    EXPECT_EQ(1, colData->elementsB.size());

    // That element should be a point directional element
    EXPECT_EQ(CollisionElementType::PointDirection, colData->elementsA[0].m_type);
    EXPECT_EQ(CollisionElementType::PointDirection, colData->elementsB[0].m_type);

    // Sphere A (which is below B), should be pushed down, while sphere B (above A), should be pushed up
    EXPECT_EQ(Vec3d(0.0, 1.0, 0.0), colData->elementsA[0].m_element.m_PointDirectionElement.dir);
    EXPECT_EQ(Vec3d(0.0, -1.0, 0.0), colData->elementsB[0].m_element.m_PointDirectionElement.dir);

    // Should have depths of 0.5
    EXPECT_EQ(0.25, colData->elementsA[0].m_element.m_PointDirectionElement.penetrationDepth);
    EXPECT_EQ(0.25, colData->elementsB[0].m_element.m_PointDirectionElement.penetrationDepth);

    // Contacts should be on the surface of spheres
    EXPECT_EQ(Vec3d(0.0, 0.25, 0.0), colData->elementsA[0].m_element.m_PointDirectionElement.pt);
    EXPECT_EQ(Vec3d(0.0, 0.5, 0.0), colData->elementsB[0].m_element.m_PointDirectionElement.pt);
}

TEST(imstkSphereToCylinderCDTest, NonIntersectionTestAB)
{
    auto sphere   = std::make_shared<Sphere>(Vec3d(0.0, 1.0, 0.0), 0.4);
    auto cylinder = std::make_shared<Cylinder>();

    SphereToCylinderCD m_sphereToCylinderCD;
    m_sphereToCylinderCD.setInput(sphere, 0);
    m_sphereToCylinderCD.setInput(cylinder, 1);
    m_sphereToCylinderCD.setGenerateCD(true, true); // Generate both A and B
    m_sphereToCylinderCD.update();

    std::shared_ptr<CollisionData> colData = m_sphereToCylinderCD.getCollisionData();

    // Should be no elements
    EXPECT_EQ(0, colData->elementsA.size());
    EXPECT_EQ(0, colData->elementsB.size());
}