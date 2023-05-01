/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCapsule.h"
#include "imstkSurfaceMesh.h"
#include "imstkClosedSurfaceMeshToCapsuleCD.h"
#include "imstkVecDataArray.h"

#include <gtest/gtest.h>

using namespace imstk;

// Test for intersection with a vertex on the shell of the capsule
/*

  ______
 \      /
  \    /
   \  /
    \/
    ***
   / ^ \
   | | |
   | *-|-->
   |   |
   \   /
    ***

*/
TEST(imstkClosedSurfaceMeshToCapsuleCDTest, IntersectionTestAB_VertexShell)
{
    // Vertex-to-capsule point case
    auto capsule = std::make_shared<Capsule>(Vec3d::Zero(), 0.5, 1.0);

    // Create surafce mesh (single triangle)
    auto surfMesh    = std::make_shared<SurfaceMesh>();
    auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(3);

    // Point locations on triangle
    (*verticesPtr)[0] = Vec3d(0.0, 1.0, 0.0); // Contact point
    (*verticesPtr)[1] = Vec3d(1.0, 2.0, 0.0);
    (*verticesPtr)[2] = Vec3d(-1.0, 2.0, -1.0);

    auto indicesPtr = std::make_shared<VecDataArray<int, 3>>(1);
    (*indicesPtr)[0] = Vec3i(0, 1, 2);
    surfMesh->initialize(verticesPtr, indicesPtr);

    // manually test for collision
    ClosedSurfaceMeshToCapsuleCD colDetect;

    colDetect.setInput(surfMesh, 0);
    colDetect.setInput(capsule, 1);
    colDetect.setGenerateCD(true, true);
    colDetect.update();

    std::shared_ptr<CollisionData> colData = colDetect.getCollisionData();

    EXPECT_EQ(1, colData->elementsA.size());
    EXPECT_EQ(1, colData->elementsB.size());

    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsA[0].m_type);
    EXPECT_EQ(CollisionElementType::PointDirection, colData->elementsB[0].m_type);
}

// Test for intersection with a vertex on the interior
TEST(imstkClosedSurfaceMeshToCapsuleCDTest, IntersectionTestAB_VertexInterior)
{
    // Vertex-to-capsule point case
    auto capsule = std::make_shared<Capsule>(Vec3d::Zero(), 0.5, 1.0);

    // Create surafce mesh (single triangle)
    auto surfMesh    = std::make_shared<SurfaceMesh>();
    auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(3);

    // Point locations on triangle
    (*verticesPtr)[0] = Vec3d(0.0, 0.8, 0.0); // Contact point (interior)
    (*verticesPtr)[1] = Vec3d(1.0, 2.0, 0.0);
    (*verticesPtr)[2] = Vec3d(-1.0, 2.0, -1.0);

    auto indicesPtr = std::make_shared<VecDataArray<int, 3>>(1);
    (*indicesPtr)[0] = Vec3i(0, 1, 2);
    surfMesh->initialize(verticesPtr, indicesPtr);

    // manually test for collision
    ClosedSurfaceMeshToCapsuleCD colDetect;

    colDetect.setInput(surfMesh, 0);
    colDetect.setInput(capsule, 1);
    colDetect.setGenerateCD(true, true);
    colDetect.update();

    std::shared_ptr<CollisionData> colData = colDetect.getCollisionData();

    EXPECT_EQ(1, colData->elementsA.size());
    EXPECT_EQ(1, colData->elementsB.size());

    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsA[0].m_type);
    EXPECT_EQ(CollisionElementType::PointDirection, colData->elementsB[0].m_type);
}

// Test for interseaction with an edge on the surface of the capsule
/*
     *
    / \
   /   \
  /_____\
    ***
   / ^ \
   | | |
   | *-|-->
   |   |
   \   /
    ***
*/
TEST(imstkClosedSurfaceMeshToCapsuleCDTest, IntersectionTestAB_EdgeShell)
{
    // Vertex-to-capsule edge case
    auto capsule = std::make_shared<Capsule>(Vec3d::Zero(), 0.5, 1.0);

    // Create surafce mesh (single triangle)
    auto surfMesh    = std::make_shared<SurfaceMesh>();
    auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(3);
    (*verticesPtr)[0] = Vec3d(-1.0, 1.0, 0.0);
    (*verticesPtr)[1] = Vec3d(1.0, 1.0, 0.0);
    (*verticesPtr)[2] = Vec3d(0.0, 2.0, 0.0);
    auto indicesPtr = std::make_shared<VecDataArray<int, 3>>(1);
    (*indicesPtr)[0] = Vec3i(0, 1, 2);
    surfMesh->initialize(verticesPtr, indicesPtr);

    // manually test for collision
    ClosedSurfaceMeshToCapsuleCD colDetect;
    colDetect.setInput(surfMesh, 0);
    colDetect.setInput(capsule, 1);
    colDetect.setGenerateCD(true, true);
    colDetect.update();

    std::shared_ptr<CollisionData> colData = colDetect.getCollisionData();

    EXPECT_EQ(1, colData->elementsA.size());
    EXPECT_EQ(1, colData->elementsB.size());

    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsA[0].m_type);
    EXPECT_EQ(CollisionElementType::PointDirection, colData->elementsB[0].m_type);

    EXPECT_EQ(IMSTK_TRIANGLE, colData->elementsA[0].m_element.m_CellIndexElement.cellType);
}

// Test for interseaction with an edge on the interior of the capsule
TEST(imstkClosedSurfaceMeshToCapsuleCDTest, IntersectionTestAB_EdgeInterior)
{
    // Vertex-to-capsule edge case
    auto capsule = std::make_shared<Capsule>(Vec3d::Zero(), 0.5, 1.0);

    // Create surafce mesh (single triangle)
    auto surfMesh    = std::make_shared<SurfaceMesh>();
    auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(3);
    (*verticesPtr)[0] = Vec3d(-1.0, 0.9, 0.0);
    (*verticesPtr)[1] = Vec3d(1.0, 0.9, 0.0);
    (*verticesPtr)[2] = Vec3d(0.0, 2.0, 0.0);
    auto indicesPtr = std::make_shared<VecDataArray<int, 3>>(1);
    (*indicesPtr)[0] = Vec3i(0, 1, 2);
    surfMesh->initialize(verticesPtr, indicesPtr);

    // manually test for collision
    ClosedSurfaceMeshToCapsuleCD colDetect;
    colDetect.setInput(surfMesh, 0);
    colDetect.setInput(capsule, 1);
    colDetect.setGenerateCD(true, true);
    colDetect.update();

    std::shared_ptr<CollisionData> colData = colDetect.getCollisionData();

    EXPECT_EQ(1, colData->elementsA.size());
    EXPECT_EQ(1, colData->elementsB.size());

    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsA[0].m_type);
    EXPECT_EQ(CollisionElementType::PointDirection, colData->elementsB[0].m_type);

    EXPECT_EQ(IMSTK_TRIANGLE, colData->elementsA[0].m_element.m_CellIndexElement.cellType);
}

// Test for intersection with a face on the shell
TEST(imstkClosedSurfaceMeshToCapsuleCDTest, IntersectionTestAB_FaceShell)
{
    // Vertex-to-capsule edge face case
    auto capsule = std::make_shared<Capsule>(Vec3d::Zero(), 0.5, 1.0);

    // Create surafce mesh (single triangle)
    auto surfMesh    = std::make_shared<SurfaceMesh>();
    auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(3);
    (*verticesPtr)[0] = Vec3d(-1.0, 1.0, -1.0);
    (*verticesPtr)[1] = Vec3d(1.0, 1.0, -1.0);
    (*verticesPtr)[2] = Vec3d(0.0, 1.0, 1.0);
    auto indicesPtr = std::make_shared<VecDataArray<int, 3>>(1);
    (*indicesPtr)[0] = Vec3i(0, 1, 2);
    surfMesh->initialize(verticesPtr, indicesPtr);

    // manually test for collision
    ClosedSurfaceMeshToCapsuleCD colDetect;
    colDetect.setInput(surfMesh, 0);
    colDetect.setInput(capsule, 1);
    colDetect.setGenerateCD(true, true);
    colDetect.update();

    std::shared_ptr<CollisionData> colData = colDetect.getCollisionData();

    EXPECT_EQ(1, colData->elementsA.size());
    EXPECT_EQ(1, colData->elementsB.size());

    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsA[0].m_type);
    EXPECT_EQ(CollisionElementType::PointDirection, colData->elementsB[0].m_type);

    EXPECT_EQ(IMSTK_TRIANGLE, colData->elementsA[0].m_element.m_CellIndexElement.cellType);
}

///
/// \brief Test for intersection when only the face is in contact with the
/// tip/sphere of the capsule
///
TEST(imstkClosedSurfaceMeshToCapsuleCDTest, IntersectionTestAB_FaceInteriorEnd)
{
    // Vertex-to-capsule edge face case
    auto capsule = std::make_shared<Capsule>(Vec3d::Zero(), 0.5, 1.0);

    // Create surafce mesh (single triangle)
    auto surfMesh    = std::make_shared<SurfaceMesh>();
    auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(3);
    (*verticesPtr)[0] = Vec3d(-1.0, 0.9, -1.0);
    (*verticesPtr)[1] = Vec3d(1.0, 0.9, -1.0);
    (*verticesPtr)[2] = Vec3d(0.0, 0.9, 1.0);
    auto indicesPtr = std::make_shared<VecDataArray<int, 3>>(1);
    (*indicesPtr)[0] = Vec3i(0, 1, 2);
    surfMesh->initialize(verticesPtr, indicesPtr);

    // manually test for collision
    ClosedSurfaceMeshToCapsuleCD colDetect;
    colDetect.setInput(surfMesh, 0);
    colDetect.setInput(capsule, 1);
    colDetect.setGenerateCD(true, true);
    colDetect.update();

    std::shared_ptr<CollisionData> colData = colDetect.getCollisionData();

    EXPECT_EQ(1, colData->elementsA.size());
    EXPECT_EQ(1, colData->elementsB.size());

    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsA[0].m_type);
    EXPECT_EQ(CollisionElementType::PointDirection, colData->elementsB[0].m_type);

    EXPECT_EQ(IMSTK_TRIANGLE, colData->elementsA[0].m_element.m_CellIndexElement.cellType);
}

///
/// \brief Test for intersection when only the face is in contact with the
/// midpiece/cylinder of the capsule
/// \todo: Edge case not currently implemented in SurfaceMeshToCapsuleCD
///
TEST(imstkClosedSurfaceMeshToCapsuleCDTest, IntersectionTestAB_FaceInteriorMidpiece)
{
    // Vertex-to-capsule edge face case
    auto capsule = std::make_shared<Capsule>(Vec3d(0.0, 0.1, 0.0), 0.1, 0.5);

    // Create surafce mesh (single triangle)
    auto                    surfMesh = std::make_shared<SurfaceMesh>();
    VecDataArray<double, 3> triangleVertices(3);
    triangleVertices[0] = Vec3d(0.5, 0.0, -1.0 / 3.0);
    triangleVertices[1] = Vec3d(-0.5, 0.1, -1.0 / 3.0);
    triangleVertices[2] = Vec3d(0.0, 0.0, 2.0 / 3.0);

    VecDataArray<int, 3> triangleIndices(1);
    triangleIndices[0] = Vec3i(0, 1, 2);
    surfMesh->initialize(
        std::make_shared<VecDataArray<double, 3>>(triangleVertices),
        std::make_shared<VecDataArray<int, 3>>(triangleIndices));

    // manually test for collision
    ClosedSurfaceMeshToCapsuleCD colDetect;
    colDetect.setInput(surfMesh, 0);
    colDetect.setInput(capsule, 1);
    colDetect.setGenerateCD(true, true);
    colDetect.update();

    std::shared_ptr<CollisionData> colData = colDetect.getCollisionData();

    EXPECT_EQ(1, colData->elementsA.size());
    EXPECT_EQ(1, colData->elementsB.size());

    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsA[0].m_type);
    EXPECT_EQ(CollisionElementType::PointDirection, colData->elementsB[0].m_type);

    EXPECT_EQ(IMSTK_TRIANGLE, colData->elementsA[0].m_element.m_CellIndexElement.cellType);
}

// Test for no intersection
TEST(imstkClosedSurfaceMeshToCapsuleCDTest, NonIntersectionTestAB)
{
    // Vertex-to-capsule no intersection case
    auto capsule = std::make_shared<Capsule>(Vec3d::Zero(), 0.5, 1.0);

    // Create surafce mesh (single triangle)
    auto surfMesh    = std::make_shared<SurfaceMesh>();
    auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(3);
    (*verticesPtr)[0] = Vec3d(-1.0, 2.0, -1.0);
    (*verticesPtr)[1] = Vec3d(1.0, 2.0, -1.0);
    (*verticesPtr)[2] = Vec3d(0.0, 2.0, 1.0);
    auto indicesPtr = std::make_shared<VecDataArray<int, 3>>(1);
    (*indicesPtr)[0] = Vec3i(0, 1, 2);
    surfMesh->initialize(verticesPtr, indicesPtr);

    // manually test for collision
    ClosedSurfaceMeshToCapsuleCD colDetect;
    colDetect.setInput(surfMesh, 0);
    colDetect.setInput(capsule, 1);
    colDetect.setGenerateCD(true, true);
    colDetect.update();

    std::shared_ptr<CollisionData> colData = colDetect.getCollisionData();

    EXPECT_EQ(0, colData->elementsA.size());
    EXPECT_EQ(0, colData->elementsB.size());
}