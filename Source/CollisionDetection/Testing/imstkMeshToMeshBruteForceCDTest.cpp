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

#include "gtest/gtest.h"

#include "imstkOrientedBox.h"
#include "imstkSurfaceMesh.h"
#include "imstkMeshToMeshBruteForceCD.h"
#include "imstkGeometryUtilities.h"

using namespace imstk;

///
/// \brief TODO
///
class imstkMeshToMeshBruteForceCDTest : public ::testing::Test
{
protected:
    MeshToMeshBruteForceCD m_meshToMeshBruteForceCD;
};

TEST_F(imstkMeshToMeshBruteForceCDTest, IntersectionTestAB_EdgeToEdge)
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

    m_meshToMeshBruteForceCD.setInput(box1Mesh, 0);
    m_meshToMeshBruteForceCD.setInput(box2Mesh, 1);
    m_meshToMeshBruteForceCD.setGenerateCD(true, true); // Generate both A and B
    m_meshToMeshBruteForceCD.setGenerateEdgeEdgeContacts(true);
    m_meshToMeshBruteForceCD.update();

    std::shared_ptr<CollisionData> colData = m_meshToMeshBruteForceCD.getCollisionData();

    // Check for a single edge vs edge
    ASSERT_EQ(1, colData->elementsA.getSize());
    ASSERT_EQ(1, colData->elementsB.getSize());

    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsA[0].m_type);
    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsB[0].m_type);

    EXPECT_EQ(colData->elementsA[0].m_element.m_CellIndexElement.cellType, IMSTK_EDGE);
    EXPECT_EQ(colData->elementsB[0].m_element.m_CellIndexElement.cellType, IMSTK_EDGE);

    EXPECT_EQ(colData->elementsA[0].m_element.m_CellIndexElement.idCount, 2);
    EXPECT_EQ(colData->elementsB[0].m_element.m_CellIndexElement.idCount, 2);
}

TEST_F(imstkMeshToMeshBruteForceCDTest, IntersectionTestAB_VertexToTriangle)
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

    m_meshToMeshBruteForceCD.setInput(triMesh, 0);
    m_meshToMeshBruteForceCD.setInput(vertexMesh, 1);
    m_meshToMeshBruteForceCD.setGenerateCD(true, true); // Generate both A and B
    m_meshToMeshBruteForceCD.setGenerateEdgeEdgeContacts(true);
    m_meshToMeshBruteForceCD.update();

    std::shared_ptr<CollisionData> colData = m_meshToMeshBruteForceCD.getCollisionData();

    // Check for a single vertex-triangle case
    ASSERT_EQ(1, colData->elementsA.getSize());
    ASSERT_EQ(1, colData->elementsB.getSize());

    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsA[0].m_type);
    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsB[0].m_type);

    EXPECT_EQ(colData->elementsA[0].m_element.m_CellIndexElement.cellType, IMSTK_TRIANGLE);
    EXPECT_EQ(colData->elementsB[0].m_element.m_CellIndexElement.cellType, IMSTK_VERTEX);

    EXPECT_EQ(colData->elementsA[0].m_element.m_CellIndexElement.idCount, 3);
    EXPECT_EQ(colData->elementsB[0].m_element.m_CellIndexElement.idCount, 1);
}

TEST_F(imstkMeshToMeshBruteForceCDTest, IntersectionTestAB_VertexToVertex)
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

    m_meshToMeshBruteForceCD.setInput(triMesh, 0);
    m_meshToMeshBruteForceCD.setInput(vertexMesh, 1);
    m_meshToMeshBruteForceCD.setGenerateCD(true, true); // Generate both A and B
    m_meshToMeshBruteForceCD.setGenerateEdgeEdgeContacts(true);
    m_meshToMeshBruteForceCD.update();

    std::shared_ptr<CollisionData> colData = m_meshToMeshBruteForceCD.getCollisionData();

    // Check for a single vertex-triangle case
    ASSERT_EQ(1, colData->elementsA.getSize());
    ASSERT_EQ(1, colData->elementsB.getSize());

    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsA[0].m_type);
    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsB[0].m_type);

    EXPECT_EQ(colData->elementsA[0].m_element.m_CellIndexElement.cellType, IMSTK_VERTEX);
    EXPECT_EQ(colData->elementsB[0].m_element.m_CellIndexElement.cellType, IMSTK_VERTEX);

    EXPECT_EQ(colData->elementsA[0].m_element.m_CellIndexElement.idCount, 1);
    EXPECT_EQ(colData->elementsB[0].m_element.m_CellIndexElement.idCount, 1);
}

TEST_F(imstkMeshToMeshBruteForceCDTest, IntersectionTestAB_VertexToEdge)
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

    m_meshToMeshBruteForceCD.setInput(triMesh, 0);
    m_meshToMeshBruteForceCD.setInput(vertexMesh, 1);
    m_meshToMeshBruteForceCD.setGenerateCD(true, true); // Generate both A and B
    m_meshToMeshBruteForceCD.setGenerateEdgeEdgeContacts(true);
    m_meshToMeshBruteForceCD.update();

    std::shared_ptr<CollisionData> colData = m_meshToMeshBruteForceCD.getCollisionData();

    // Check for a single vertex-triangle case
    ASSERT_EQ(1, colData->elementsA.getSize());
    ASSERT_EQ(1, colData->elementsB.getSize());

    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsA[0].m_type);
    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsB[0].m_type);

    EXPECT_EQ(colData->elementsA[0].m_element.m_CellIndexElement.cellType, IMSTK_EDGE);
    EXPECT_EQ(colData->elementsB[0].m_element.m_CellIndexElement.cellType, IMSTK_VERTEX);

    EXPECT_EQ(colData->elementsA[0].m_element.m_CellIndexElement.idCount, 2);
    EXPECT_EQ(colData->elementsB[0].m_element.m_CellIndexElement.idCount, 1);
}

int
imstkMeshToMeshBruteForceCDTest(int argc, char* argv[])
{
    // Init Google Test & Mock
    ::testing::InitGoogleTest(&argc, argv);

    // Run tests with gtest
    return RUN_ALL_TESTS();
}
