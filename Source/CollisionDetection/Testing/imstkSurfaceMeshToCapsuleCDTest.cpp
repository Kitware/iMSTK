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

#include "imstkCapsule.h"
#include "imstkSurfaceMesh.h"
#include "imstkSurfaceMeshToCapsuleCD.h"
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
TEST(imstkSurfaceMeshToCapsuleCDTest, IntersectionTestAB_VertexShell)
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
    SurfaceMeshToCapsuleCD colDetect;

    colDetect.setInput(surfMesh, 0);
    colDetect.setInput(capsule, 1);
    colDetect.setGenerateCD(true, true);
    colDetect.update();

    std::shared_ptr<CollisionData> colData = colDetect.getCollisionData();

    EXPECT_EQ(1, colData->elementsA.size());
    EXPECT_EQ(1, colData->elementsB.size());

    EXPECT_EQ(CollisionElementType::PointIndexDirection, colData->elementsA[0].m_type);
    EXPECT_EQ(CollisionElementType::PointDirection, colData->elementsB[0].m_type);
}

// Test for intersection with a vertex on the interior
TEST(imstkSurfaceMeshToCapsuleCDTest, IntersectionTestAB_VertexInterior)
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
    SurfaceMeshToCapsuleCD colDetect;

    colDetect.setInput(surfMesh, 0);
    colDetect.setInput(capsule, 1);
    colDetect.setGenerateCD(true, true);
    colDetect.update();

    std::shared_ptr<CollisionData> colData = colDetect.getCollisionData();

    EXPECT_EQ(1, colData->elementsA.size());
    EXPECT_EQ(1, colData->elementsB.size());

    EXPECT_EQ(CollisionElementType::PointIndexDirection, colData->elementsA[0].m_type);
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
TEST(imstkSurfaceMeshToCapsuleCDTest, IntersectionTestAB_EdgeShell)
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
    SurfaceMeshToCapsuleCD colDetect;
    colDetect.setInput(surfMesh, 0);
    colDetect.setInput(capsule, 1);
    colDetect.setGenerateCD(true, true);
    colDetect.update();

    std::shared_ptr<CollisionData> colData = colDetect.getCollisionData();

    EXPECT_EQ(1, colData->elementsA.size());
    EXPECT_EQ(1, colData->elementsB.size());

    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsA[0].m_type);
    EXPECT_EQ(CollisionElementType::PointDirection, colData->elementsB[0].m_type);

    EXPECT_EQ(IMSTK_EDGE, colData->elementsA[0].m_element.m_CellIndexElement.cellType);
}

// Test for interseaction with an edge on the interior of the capsule
TEST(imstkSurfaceMeshToCapsuleCDTest, IntersectionTestAB_EdgeInterior)
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
    SurfaceMeshToCapsuleCD colDetect;
    colDetect.setInput(surfMesh, 0);
    colDetect.setInput(capsule, 1);
    colDetect.setGenerateCD(true, true);
    colDetect.update();

    std::shared_ptr<CollisionData> colData = colDetect.getCollisionData();

    EXPECT_EQ(1, colData->elementsA.size());
    EXPECT_EQ(1, colData->elementsB.size());

    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsA[0].m_type);
    EXPECT_EQ(CollisionElementType::PointDirection, colData->elementsB[0].m_type);

    EXPECT_EQ(IMSTK_EDGE, colData->elementsA[0].m_element.m_CellIndexElement.cellType);
}

// Test for interseaction with a face on the shell
TEST(imstkSurfaceMeshToCapsuleCDTest, IntersectionTestAB_FaceShell)
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
    SurfaceMeshToCapsuleCD colDetect;
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

// Test for interseaction with a face on the interior
TEST(imstkSurfaceMeshToCapsuleCDTest, IntersectionTestAB_FaceInterior)
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
    SurfaceMeshToCapsuleCD colDetect;
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
TEST(imstkSurfaceMeshToCapsuleCDTest, NonIntersectionTestAB)
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
    SurfaceMeshToCapsuleCD colDetect;
    colDetect.setInput(surfMesh, 0);
    colDetect.setInput(capsule, 1);
    colDetect.setGenerateCD(true, true);
    colDetect.update();

    std::shared_ptr<CollisionData> colData = colDetect.getCollisionData();

    EXPECT_EQ(0, colData->elementsA.size());
    EXPECT_EQ(0, colData->elementsB.size());
}