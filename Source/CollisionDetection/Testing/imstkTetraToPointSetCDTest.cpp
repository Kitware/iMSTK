/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "gtest/gtest.h"

#include "imstkPointSet.h"
#include "imstkTetrahedralMesh.h"
#include "imstkTetraToPointSetCD.h"

using namespace imstk;

std::shared_ptr<TetrahedralMesh>
makeUnitTetrahedron_()
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

TEST(imstkTetraToPointSetCDTest, IntersectionTestAB)
{
    // Create tetrahedron
    auto tetMesh = makeUnitTetrahedron_();

    // Create point set
    auto pointSet = std::make_shared<PointSet>();
    auto verxPtr  = std::make_shared<VecDataArray<double, 3>>(1);
    (*verxPtr)[0] = Vec3d(0.0, 0.0, 0.0);
    double baryCoord[4] = { 0.2, 0.3, 0.1, 0.4 };// All non-negative and should sum to 1

    // Create a point inside the tetrahedron from the barycentric coordinates
    for (size_t i = 0; i < 4; ++i)
    {
        (*verxPtr)[0] += baryCoord[i] * tetMesh->getVertexPosition(i);
    }

    pointSet->initialize(verxPtr);

    // create collision
    TetraToPointSetCD m_tetraToPointSetCD;

    m_tetraToPointSetCD.setInput(pointSet, 0);
    m_tetraToPointSetCD.setInput(tetMesh, 1);
    m_tetraToPointSetCD.setGenerateCD(true, true); // Generate both A and B
    m_tetraToPointSetCD.update();

    std::shared_ptr<CollisionData> colData = m_tetraToPointSetCD.getCollisionData();

    // Should be one element on side A, 0 on side B (default CD data is not generated for the sphere)
    EXPECT_EQ(1, colData->elementsA.size());
    EXPECT_EQ(1, colData->elementsB.size());

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
    auto tetMesh = makeUnitTetrahedron_();

    // Create point set
    auto pointSet = std::make_shared<PointSet>();
    auto verxPtr  = std::make_shared<VecDataArray<double, 3>>(1);
    (*verxPtr)[0] = Vec3d(1.0, 1.0, 1.0);// some point outside
    pointSet->initialize(verxPtr);

    // create collision
    TetraToPointSetCD m_tetraToPointSetCD;

    m_tetraToPointSetCD.setInput(pointSet, 0);
    m_tetraToPointSetCD.setInput(tetMesh, 1);
    m_tetraToPointSetCD.setGenerateCD(true, true); // Generate both A and B
    m_tetraToPointSetCD.update();

    std::shared_ptr<CollisionData> colData = m_tetraToPointSetCD.getCollisionData();

    // Should have no elements
    EXPECT_EQ(0, colData->elementsA.size());
    EXPECT_EQ(0, colData->elementsB.size());
}