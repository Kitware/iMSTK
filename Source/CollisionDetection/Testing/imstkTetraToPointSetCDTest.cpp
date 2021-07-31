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

#include "imstkPointSet.h"
#include "imstkTetrahedralMesh.h"
#include "imstkTetraToPointSetCD.h"

using namespace imstk;

TEST(imstkTetraToPointSetCDTest, IntersectionTestAB)
{
    // Create tetrahedron
    auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(4);
    auto indicesPtr  = std::make_shared<VecDataArray<int, 4>>(1);

    VecDataArray<double, 3>& vertices = *verticesPtr;
    VecDataArray<int, 4>&    indices  = *indicesPtr;

    vertices[0] = Vec3d(0.0, 0.0, 0.0);
    vertices[1] = Vec3d(1.0, 0.0, 0.0);
    vertices[2] = Vec3d(0.0, 1.0, 0.0);
    vertices[3] = Vec3d(0.0, 0.0, 1.0);

    indices[0] = Vec4i(0, 1, 2, 3);

    auto tetMesh = std::make_shared<TetrahedralMesh>();
    tetMesh->initialize(verticesPtr, indicesPtr);

    // Create point set
    auto pointSet = std::make_shared<PointSet>();
    auto verxPtr  = std::make_shared<VecDataArray<double, 3>>(1);
    (*verxPtr)[0] = Vec3d(0.0, 0.0, 0.0);
    double baryCoord[4] = { 0.2, 0.3, 0.1, 0.4 };// All non-negative and should sum to 1

    // Create a point inside the tetrahedron from the barycentric coordinates
    for (int i = 0; i < 4; ++i)
    {
        (*verxPtr)[0] += baryCoord[i] * vertices[i];
                                                                               }

    pointSet->initialize(verxPtr);

    std::cout << (*verxPtr)[0];

    // create collision
    TetraToPointSetCD m_tetraToPointSetCD;

    m_tetraToPointSetCD.setInput(pointSet, 0);
    m_tetraToPointSetCD.setInput(tetMesh, 1);
    m_tetraToPointSetCD.setGenerateCD(true, true); // Generate both A and B
    m_tetraToPointSetCD.update();

    std::shared_ptr<CollisionData> colData = m_tetraToPointSetCD.getCollisionData();

    // Should be one element on side A, 0 on side B (default CD data is not generated for the sphere)
    EXPECT_EQ(1, colData->elementsA.getSize());
    EXPECT_EQ(1, colData->elementsB.getSize());

    // That element should be a point directional element
    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsA[0].m_type);
    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsB[0].m_type);

    // Check cell types
    EXPECT_EQ(IMSTK_VERTEX, colData->elementsA[0].m_element.m_CellIndexElement.cellType);
    EXPECT_EQ(IMSTK_TETRAHEDRON, colData->elementsB[0].m_element.m_CellIndexElement.cellType);

    // Check cell types
    EXPECT_EQ(0, colData->elementsA[0].m_element.m_CellIndexElement.ids[0]);
    EXPECT_EQ(0, colData->elementsB[0].m_element.m_CellIndexElement.ids[0]);
}

TEST(imstkTetraToPointSetCDTest, NonIntersectionTestAB)
{
    // Create tetrahedron
    auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(4);
    auto indicesPtr  = std::make_shared<VecDataArray<int, 4>>(1);

    VecDataArray<double, 3>& vertices = *verticesPtr;
    VecDataArray<int, 4>&    indices  = *indicesPtr;

    vertices[0] = Vec3d(0.0, 0.0, 0.0);
    vertices[1] = Vec3d(1.0, 0.0, 0.0);
    vertices[2] = Vec3d(0.0, 1.0, 0.0);
    vertices[3] = Vec3d(0.0, 0.0, 1.0);

    indices[0] = Vec4i(0, 1, 2, 3);

    auto tetMesh = std::make_shared<TetrahedralMesh>();
    tetMesh->initialize(verticesPtr, indicesPtr);

    // Create point set
    auto pointSet = std::make_shared<PointSet>();
    auto verxPtr  = std::make_shared<VecDataArray<double, 3>>(1);
    (*verxPtr)[0] = Vec3d(1.0, 1.0, 1.0);
    pointSet->initialize(verxPtr);

    // create collision
    TetraToPointSetCD m_tetraToPointSetCD;

    m_tetraToPointSetCD.setInput(pointSet, 0);
    m_tetraToPointSetCD.setInput(tetMesh, 1);
    m_tetraToPointSetCD.setGenerateCD(true, true); // Generate both A and B
    m_tetraToPointSetCD.update();

    std::shared_ptr<CollisionData> colData = m_tetraToPointSetCD.getCollisionData();

    // Should have no elements
    EXPECT_EQ(0, colData->elementsA.getSize());
    EXPECT_EQ(0, colData->elementsB.getSize());
}

int
imstkTetraToPointSetCDTest(int argc, char* argv[])
{
    // Init Google Test & Mock
    ::testing::InitGoogleTest(&argc, argv);

    // Run tests with gtest
    return RUN_ALL_TESTS();
}
