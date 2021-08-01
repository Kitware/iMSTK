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

#include "imstkLineMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkTetraToLineMeshCD.h"

using namespace imstk;

std::shared_ptr<TetrahedralMesh>
makeUnitTetrahedron()
{
    // Create tetrahedron
    auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(4);
    auto indicesPtr  = std::make_shared<VecDataArray<int, 4>>(1);

    (*verticesPtr)[0] = Vec3d(0.0, 0.0, 0.0);
    (*verticesPtr)[1] = Vec3d(1.0, 0.0, 0.0);
    (*verticesPtr)[2] = Vec3d(0.0, 1.0, 0.0);
    (*verticesPtr)[3] = Vec3d(0.0, 0.0, 1.0);

    (*indicesPtr)[0] = Vec4i(0, 1, 2, 3);

    auto tetMesh = std::make_shared<TetrahedralMesh>();
    tetMesh->initialize(verticesPtr, indicesPtr);

    return tetMesh;
}

std::shared_ptr<LineMesh>
makeOneSegmentLineMesh(const Vec3d& a, const Vec3d& b)
{
    auto lineMesh   = std::make_shared<LineMesh>();
    auto verxPtr    = std::make_shared<VecDataArray<double, 3>>(2);
    auto indicesPtr = std::make_shared<VecDataArray<int, 2>>(1);
    (*verxPtr)[0]    = a;
    (*verxPtr)[1]    = b;
    (*indicesPtr)[0] = Vec2i(0, 1);

    lineMesh->initialize(verxPtr, indicesPtr);

    return lineMesh;
}

TEST(imstkTetraToLineMeshCDTest, IntersectionTestAB_enclosed)
{
    // Create tetrahedron
    auto tetMesh = makeUnitTetrahedron();

    // Create enclosed line mesh
    auto lineMesh = makeOneSegmentLineMesh(Vec3d(0.05, 0.05, 0.05), Vec3d(0.07, 0.07, 0.07));

    // create collision
    TetraToLineMeshCD m_tetraToLineMeshCD;

    m_tetraToLineMeshCD.setInput(lineMesh, 0);
    m_tetraToLineMeshCD.setInput(tetMesh, 1);
    m_tetraToLineMeshCD.setGenerateCD(true, true); // Generate both A and B
    m_tetraToLineMeshCD.update();

    std::shared_ptr<CollisionData> colData = m_tetraToLineMeshCD.getCollisionData();

    // Should be one element on side A, 1 on side B
    EXPECT_EQ(1, colData->elementsA.getSize());
    EXPECT_EQ(1, colData->elementsB.getSize());

    // That element should be a point directional element
    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsA[0].m_type);
    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsB[0].m_type);

    // Check cell types
    EXPECT_EQ(IMSTK_EDGE, colData->elementsA[0].m_element.m_CellIndexElement.cellType);
    EXPECT_EQ(IMSTK_TETRAHEDRON, colData->elementsB[0].m_element.m_CellIndexElement.cellType);

    // Check cell types
    EXPECT_EQ(0, colData->elementsA[0].m_element.m_CellIndexElement.ids[0]);
    EXPECT_EQ(0, colData->elementsB[0].m_element.m_CellIndexElement.ids[0]);
}

TEST(imstkTetraToLineMeshCDTest, IntersectionTestAB_intersecting)
{
    // Create tetrahedron
    auto tetMesh = makeUnitTetrahedron();

    // Create intersecting line mesh
    auto lineMesh = makeOneSegmentLineMesh(Vec3d(-1.0, -1.0, -1.0), Vec3d(1.0, 1.0, 1.0));

    // create collision
    TetraToLineMeshCD m_tetraToLineMeshCD;

    m_tetraToLineMeshCD.setInput(lineMesh, 0);
    m_tetraToLineMeshCD.setInput(tetMesh, 1);
    m_tetraToLineMeshCD.setGenerateCD(true, true); // Generate both A and B
    m_tetraToLineMeshCD.update();

    std::shared_ptr<CollisionData> colData = m_tetraToLineMeshCD.getCollisionData();

    // Should be one element on side A, 1 on side B
    EXPECT_EQ(1, colData->elementsA.getSize());
    EXPECT_EQ(1, colData->elementsB.getSize());

    // That element should be a point directional element
    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsA[0].m_type);
    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsB[0].m_type);

    // Check cell types
    EXPECT_EQ(IMSTK_EDGE, colData->elementsA[0].m_element.m_CellIndexElement.cellType);
    EXPECT_EQ(IMSTK_TETRAHEDRON, colData->elementsB[0].m_element.m_CellIndexElement.cellType);

    // Check cell types
    EXPECT_EQ(0, colData->elementsA[0].m_element.m_CellIndexElement.ids[0]);
    EXPECT_EQ(0, colData->elementsB[0].m_element.m_CellIndexElement.ids[0]);
}

TEST(imstkTetraToLineMeshCDTest, NonIntersectionTestAB)
{
    // Create unit tetrahedron
    auto tetMesh = makeUnitTetrahedron();

    // Create non-intersecting line mesh
    auto lineMesh = makeOneSegmentLineMesh(Vec3d(1.0, 1.0, 1.0), Vec3d(2.0, 2.0, 2.0));

    // create collision
    TetraToLineMeshCD m_tetraToLineMeshCD;

    m_tetraToLineMeshCD.setInput(lineMesh, 0);
    m_tetraToLineMeshCD.setInput(tetMesh, 1);
    m_tetraToLineMeshCD.setGenerateCD(true, true); // Generate both A and B
    m_tetraToLineMeshCD.update();

    std::shared_ptr<CollisionData> colData = m_tetraToLineMeshCD.getCollisionData();

    // Should have no elements
    EXPECT_EQ(0, colData->elementsA.getSize());
    EXPECT_EQ(0, colData->elementsB.getSize());
}

int
imstkTetraToLineMeshCDTest(int argc, char* argv[])
{
    // Init Google Test & Mock
    ::testing::InitGoogleTest(&argc, argv);

    // Run tests with gtest
    return RUN_ALL_TESTS();
}
