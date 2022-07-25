/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "gtest/gtest.h"

#include "imstkClosedSurfaceMeshToMeshCD.h"
#include "imstkGeometryUtilities.h"
#include "imstkOrientedBox.h"
#include "imstkSurfaceMesh.h"

using namespace imstk;

TEST(imstkClosedSurfaceMeshToMeshCDTest, IntersectionTestAB_EdgeToEdge)
{
    // Create two cubes
    auto box1 = std::make_shared<OrientedBox>(Vec3d::Zero(), Vec3d(0.5, 0.5, 0.5), Quatd::Identity());
    auto box2 = std::make_shared<OrientedBox>(Vec3d::Zero(), Vec3d(0.4, 0.4, 0.4), Quatd::Identity());

    std::shared_ptr<SurfaceMesh> box1Mesh = GeometryUtils::toSurfaceMesh(box1);
    std::shared_ptr<SurfaceMesh> box2Mesh = GeometryUtils::toSurfaceMesh(box2);
    box2Mesh->rotate(Vec3d(0.0, 0.0, 1.0), PI_2 * 0.5);
    box2Mesh->rotate(Vec3d(1.0, 0.0, 0.0), PI_2 * 0.5);
    box2Mesh->translate(Vec3d(0.0, 0.8, 0.8));
    box2Mesh->updatePostTransformData();

    ClosedSurfaceMeshToMeshCD m_meshCD;
    m_meshCD.setInput(box1Mesh, 0);
    m_meshCD.setInput(box2Mesh, 1);
    m_meshCD.setGenerateCD(true, true); // Generate both A and B
    m_meshCD.setGenerateEdgeEdgeContacts(true);
    m_meshCD.update();

    std::shared_ptr<CollisionData> colData = m_meshCD.getCollisionData();

    // Check for a single edge vs edge
    ASSERT_EQ(1, colData->elementsA.size());
    ASSERT_EQ(1, colData->elementsB.size());

    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsA[0].m_type);
    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsB[0].m_type);

    EXPECT_EQ(colData->elementsA[0].m_element.m_CellIndexElement.cellType, IMSTK_EDGE);
    EXPECT_EQ(colData->elementsB[0].m_element.m_CellIndexElement.cellType, IMSTK_EDGE);

    EXPECT_EQ(colData->elementsA[0].m_element.m_CellIndexElement.idCount, 2);
    EXPECT_EQ(colData->elementsB[0].m_element.m_CellIndexElement.idCount, 2);
}

TEST(imstkClosedSurfaceMeshToMeshCDTest, IntersectionTestAB_VertexToTriangle)
{
    // Create triangle on z plane
    auto triMesh = std::make_shared<SurfaceMesh>();
    {
        auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(3);
        (*verticesPtr)[0] = Vec3d(0.5, 0.0, -0.5);
        (*verticesPtr)[1] = Vec3d(-0.5, 0.0, -0.5);
        (*verticesPtr)[2] = Vec3d(0.0, 0.0, 0.5);
        auto indicesPtr = std::make_shared<VecDataArray<int, 3>>(1);
        (*indicesPtr)[0] = Vec3i(0, 1, 2);
        triMesh->initialize(verticesPtr, indicesPtr);
    }

    // Create a test PointSet that causes this vertex to be closest to the face of the triangle
    auto vertexMesh = std::make_shared<PointSet>();
    {
        auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(1);
        (*verticesPtr)[0] = Vec3d(0.0, -1.0, 0.0);
        vertexMesh->initialize(verticesPtr);
    }

    ClosedSurfaceMeshToMeshCD m_meshCD;
    m_meshCD.setInput(triMesh, 0);
    m_meshCD.setInput(vertexMesh, 1);
    m_meshCD.setGenerateCD(true, true); // Generate both A and B
    m_meshCD.setGenerateEdgeEdgeContacts(true);
    m_meshCD.update();

    std::shared_ptr<CollisionData> colData = m_meshCD.getCollisionData();

    // Check for a single vertex-triangle case
    ASSERT_EQ(1, colData->elementsA.size());
    ASSERT_EQ(1, colData->elementsB.size());

    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsA[0].m_type);
    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsB[0].m_type);

    EXPECT_EQ(colData->elementsA[0].m_element.m_CellIndexElement.cellType, IMSTK_TRIANGLE);
    EXPECT_EQ(colData->elementsB[0].m_element.m_CellIndexElement.cellType, IMSTK_VERTEX);

    EXPECT_EQ(colData->elementsA[0].m_element.m_CellIndexElement.idCount, 3);
    EXPECT_EQ(colData->elementsB[0].m_element.m_CellIndexElement.idCount, 1);
}

TEST(imstkClosedSurfaceMeshToMeshCDTest, IntersectionTestAB_VertexToVertex)
{
    // Create triangle on z plane
    auto triMesh = std::make_shared<SurfaceMesh>();
    {
        auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(3);
        (*verticesPtr)[0] = Vec3d(0.5, 0.0, -0.5);
        (*verticesPtr)[1] = Vec3d(-0.5, 0.0, -0.5);
        (*verticesPtr)[2] = Vec3d(0.0, 0.0, 0.5);
        auto indicesPtr = std::make_shared<VecDataArray<int, 3>>(1);
        (*indicesPtr)[0] = Vec3i(0, 1, 2);
        triMesh->initialize(verticesPtr, indicesPtr);
    }

    // Create a test PointSet that causes this vertex to be closest to the first vertex of the triangle
    auto vertexMesh = std::make_shared<PointSet>();
    {
        auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(1);
        (*verticesPtr)[0] = Vec3d(0.5, -1.0, -0.5);
        vertexMesh->initialize(verticesPtr);
    }

    ClosedSurfaceMeshToMeshCD m_meshCD;
    m_meshCD.setInput(triMesh, 0);
    m_meshCD.setInput(vertexMesh, 1);
    m_meshCD.setGenerateCD(true, true); // Generate both A and B
    m_meshCD.setGenerateEdgeEdgeContacts(true);
    m_meshCD.update();

    std::shared_ptr<CollisionData> colData = m_meshCD.getCollisionData();

    // Check for a single vertex-triangle case
    ASSERT_EQ(1, colData->elementsA.size());
    ASSERT_EQ(1, colData->elementsB.size());

    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsA[0].m_type);
    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsB[0].m_type);

    EXPECT_EQ(colData->elementsA[0].m_element.m_CellIndexElement.cellType, IMSTK_VERTEX);
    EXPECT_EQ(colData->elementsB[0].m_element.m_CellIndexElement.cellType, IMSTK_VERTEX);

    EXPECT_EQ(colData->elementsA[0].m_element.m_CellIndexElement.idCount, 1);
    EXPECT_EQ(colData->elementsB[0].m_element.m_CellIndexElement.idCount, 1);
}

TEST(imstkClosedSurfaceMeshToMeshCDTest, IntersectionTestAB_VertexToEdge)
{
    // Create triangle on z plane
    auto triMesh = std::make_shared<SurfaceMesh>();
    {
        auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(3);
        (*verticesPtr)[0] = Vec3d(0.5, 0.0, -0.5);
        (*verticesPtr)[1] = Vec3d(-0.5, 0.0, -0.5);
        (*verticesPtr)[2] = Vec3d(0.0, 0.0, 0.5);
        auto indicesPtr = std::make_shared<VecDataArray<int, 3>>(1);
        (*indicesPtr)[0] = Vec3i(0, 1, 2);
        triMesh->initialize(verticesPtr, indicesPtr);
    }

    // Create a test PointSet that causes this vertex to be closest to the edge of a cube
    auto vertexMesh = std::make_shared<PointSet>();
    {
        auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(1);
        (*verticesPtr)[0] = Vec3d(0.0, -1.0, -0.5);
        vertexMesh->initialize(verticesPtr);
    }

    ClosedSurfaceMeshToMeshCD m_meshCD;
    m_meshCD.setInput(triMesh, 0);
    m_meshCD.setInput(vertexMesh, 1);
    m_meshCD.setGenerateCD(true, true); // Generate both A and B
    m_meshCD.setGenerateEdgeEdgeContacts(true);
    m_meshCD.update();

    std::shared_ptr<CollisionData> colData = m_meshCD.getCollisionData();

    // Check for a single vertex-triangle case
    ASSERT_EQ(1, colData->elementsA.size());
    ASSERT_EQ(1, colData->elementsB.size());

    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsA[0].m_type);
    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsB[0].m_type);

    EXPECT_EQ(colData->elementsA[0].m_element.m_CellIndexElement.cellType, IMSTK_EDGE);
    EXPECT_EQ(colData->elementsB[0].m_element.m_CellIndexElement.cellType, IMSTK_VERTEX);

    EXPECT_EQ(colData->elementsA[0].m_element.m_CellIndexElement.idCount, 2);
    EXPECT_EQ(colData->elementsB[0].m_element.m_CellIndexElement.idCount, 1);
}