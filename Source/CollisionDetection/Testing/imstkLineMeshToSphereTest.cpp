/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkSphere.h"
#include "imstkLineMesh.h"
#include "imstkLineMeshToSphereCD.h"
#include "imstkVecDataArray.h"

#include <gtest/gtest.h>

using namespace imstk;

TEST(imstkLineMeshToSphereCDTest, SphereInsideVertexA)
{
    // Vertex-to-sphere case
    auto sphere = std::make_shared<Sphere>(Vec3d(0.0, 0.0, 0.0), 0.2);

    auto lineMesh = std::make_shared<LineMesh>();

    VecDataArray<double, 3> lineVertices(2);
    lineVertices[0] = Vec3d(0.1, 0.0, 0.0);
    lineVertices[1] = Vec3d(1.0, 0.0, 0.0);

    VecDataArray<int, 2> lineIndices(1);
    lineIndices[0] = Vec2i(0, 1);
    lineMesh->initialize(
        std::make_shared<VecDataArray<double, 3>>(lineVertices),
        std::make_shared<VecDataArray<int, 2>>(lineIndices));

    // manually test for collision
    LineMeshToSphereCD m_colDetect;
    m_colDetect.setInput(lineMesh, 0);
    m_colDetect.setInput(sphere, 1);
    m_colDetect.setGenerateCD(true, true);
    m_colDetect.update();

    std::shared_ptr<CollisionData> colData = m_colDetect.getCollisionData();

    EXPECT_EQ(1, colData->elementsA.size());
    EXPECT_EQ(1, colData->elementsB.size());

    EXPECT_EQ(CollisionElementType::PointIndexDirection, colData->elementsA[0].m_type);
    EXPECT_EQ(CollisionElementType::PointDirection, colData->elementsB[0].m_type);
}

TEST(imstkLineMeshToSphereCDTest, SphereInsideVertexB)
{
    // Vertex-to-sphere case
    auto sphere = std::make_shared<Sphere>(Vec3d(0.0, 0.0, 0.0), 0.2);

    auto lineMesh = std::make_shared<LineMesh>();

    VecDataArray<double, 3> lineVertices(2);
    lineVertices[0] = Vec3d(-1.0, 0.0, 0.0);
    lineVertices[1] = Vec3d(-0.1, 0.0, 0.0);

    VecDataArray<int, 2> lineIndices(1);
    lineIndices[0] = Vec2i(0, 1);
    lineMesh->initialize(
        std::make_shared<VecDataArray<double, 3>>(lineVertices),
        std::make_shared<VecDataArray<int, 2>>(lineIndices));

    // manually test for collision
    LineMeshToSphereCD m_colDetect;
    m_colDetect.setInput(lineMesh, 0);
    m_colDetect.setInput(sphere, 1);
    m_colDetect.setGenerateCD(true, true);
    m_colDetect.update();

    std::shared_ptr<CollisionData> colData = m_colDetect.getCollisionData();

    EXPECT_EQ(1, colData->elementsA.size());
    EXPECT_EQ(1, colData->elementsB.size());

    EXPECT_EQ(CollisionElementType::PointIndexDirection, colData->elementsA[0].m_type);
    EXPECT_EQ(CollisionElementType::PointDirection, colData->elementsB[0].m_type);
}

TEST(imstkLineMeshToSphereCDTest, SphereOnEdge)
{
    // Edge-to-sphere case
    auto sphere = std::make_shared<Sphere>(Vec3d(0.0, 0.25, 0.0), 0.3);

    auto lineMesh = std::make_shared<LineMesh>();

    VecDataArray<double, 3> lineVertices(2);
    lineVertices[0] = Vec3d(-0.75, 0.0, 0.0);
    lineVertices[1] = Vec3d(0.75, 0.0, 0.0);

    VecDataArray<int, 2> lineIndices(1);
    lineIndices[0] = Vec2i(0, 1);
    lineMesh->initialize(
        std::make_shared<VecDataArray<double, 3>>(lineVertices),
        std::make_shared<VecDataArray<int, 2>>(lineIndices));

    // manually test for collision
    LineMeshToSphereCD m_colDetect;
    m_colDetect.setInput(lineMesh, 0);
    m_colDetect.setInput(sphere, 1);
    m_colDetect.setGenerateCD(true, true);
    m_colDetect.update();

    std::shared_ptr<CollisionData> colData = m_colDetect.getCollisionData();

    EXPECT_EQ(1, colData->elementsA.size());
    EXPECT_EQ(1, colData->elementsB.size());

    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsA[0].m_type);
    EXPECT_EQ(CollisionElementType::PointDirection, colData->elementsB[0].m_type);

    EXPECT_EQ(IMSTK_EDGE, colData->elementsA[0].m_element.m_CellIndexElement.cellType);
}

TEST(imstkLinMeshToSphereCDTest, NonIntersectionTest)
{
    // No intersection
    auto sphere = std::make_shared<Sphere>(Vec3d(0.0, 0.25, 0.0), 0.2);

    auto lineMesh = std::make_shared<LineMesh>();

    VecDataArray<double, 3> lineVertices(2);
    lineVertices[0] = Vec3d(-0.75, 0.0, 0.0);
    lineVertices[1] = Vec3d(0.75, 0.0, 0.0);

    VecDataArray<int, 2> lineIndices(1);
    lineIndices[0] = Vec2i(0, 1);
    lineMesh->initialize(
        std::make_shared<VecDataArray<double, 3>>(lineVertices),
        std::make_shared<VecDataArray<int, 2>>(lineIndices));

    // manually test for collision
    LineMeshToSphereCD m_colDetect;
    m_colDetect.setInput(lineMesh, 0);
    m_colDetect.setInput(sphere, 1);
    m_colDetect.setGenerateCD(true, true);
    m_colDetect.update();

    std::shared_ptr<CollisionData> colData = m_colDetect.getCollisionData();

    EXPECT_EQ(0, colData->elementsA.size());
    EXPECT_EQ(0, colData->elementsB.size());
}