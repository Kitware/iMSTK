/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkLineMesh.h"
#include "imstkLineMeshToLineMeshCCD.h"
#include "imstkVecDataArray.h"

#include <gtest/gtest.h>

using namespace imstk;

// Defined in imstkTetraToLineMeshCDTest.cpp
std::shared_ptr<LineMesh> makeOneSegmentLineMesh(const Vec3d& a, const Vec3d& b);

std::shared_ptr<LineMesh>
makeLineMesh(const std::vector<Vec3d>& points)
{
    if (points.size() < 2)
    {
        return nullptr;
    }

    auto lineMesh   = std::make_shared<LineMesh>();
    auto verxPtr    = std::make_shared<VecDataArray<double, 3>>();
    auto indicesPtr = std::make_shared<VecDataArray<int, 2>>();

    // insert the first point
    verxPtr->push_back(points[0]);

    // insert remaining points
    for (size_t i = 1; i < points.size(); ++i)
    {
        verxPtr->push_back(points[i]);
        indicesPtr->push_back(Vec2i(i - 1, i));
    }

    lineMesh->initialize(verxPtr, indicesPtr);
    return lineMesh;
}

void
intersectionTest(
    std::shared_ptr<LineMesh> lineMeshA_prev,
    std::shared_ptr<LineMesh> lineMeshB_prev,
    std::shared_ptr<LineMesh> lineMeshA_curr,
    std::shared_ptr<LineMesh> lineMeshB_curr)
{
    // create collision
    LineMeshToLineMeshCCD m_lineMeshToLineMeshCCD;
    m_lineMeshToLineMeshCCD.updatePreviousTimestepGeometry(lineMeshA_prev, lineMeshB_prev);

    m_lineMeshToLineMeshCCD.setInput(lineMeshA_curr, 0);
    m_lineMeshToLineMeshCCD.setInput(lineMeshB_curr, 1);
    m_lineMeshToLineMeshCCD.setGenerateCD(true, true); // Generate both A and B
    m_lineMeshToLineMeshCCD.update();

    std::shared_ptr<CollisionData> colData = m_lineMeshToLineMeshCCD.getCollisionData();

    // Should be one element on side A, 1 on side B
    EXPECT_EQ(1, colData->elementsA.size());
    EXPECT_EQ(1, colData->elementsB.size());

    // m_ccdData flag should be true
    EXPECT_EQ(true, colData->elementsA[0].m_ccdData);
    EXPECT_EQ(true, colData->elementsB[0].m_ccdData);

    // That element should be a point directional element
    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsA[0].m_type);
    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsB[0].m_type);

    // Check cell types
    EXPECT_EQ(IMSTK_EDGE, colData->elementsA[0].m_element.m_CellIndexElement.cellType);
    EXPECT_EQ(IMSTK_EDGE, colData->elementsB[0].m_element.m_CellIndexElement.cellType);

    // Check id counts
    EXPECT_EQ(2, colData->elementsA[0].m_element.m_CellIndexElement.idCount);
    EXPECT_EQ(2, colData->elementsB[0].m_element.m_CellIndexElement.idCount);

    // Check cell ids
    EXPECT_EQ(0, colData->elementsA[0].m_element.m_CellIndexElement.ids[0]);
    EXPECT_EQ(1, colData->elementsA[0].m_element.m_CellIndexElement.ids[1]);
    EXPECT_EQ(0, colData->elementsB[0].m_element.m_CellIndexElement.ids[0]);
    EXPECT_EQ(1, colData->elementsB[0].m_element.m_CellIndexElement.ids[1]);
}

// There are three ways two lines can intersect, (1) edge-edge, (2) vertex-vertex, (3) edge-vertex.
// And, there are two ways moving lines collide: (a) crossing (b) direct overlap.
// Therefore, there are 6 cases to cover:

// --- crossing cases:
// 1(a)
TEST(imstkLineMeshToLineMeshCCDTest, IntersectionTestAB_edge_edge_crossing)
{
    auto lineMeshA_prev = makeOneSegmentLineMesh(Vec3d(0.00, 0.00, -0.01), Vec3d(0.00, 0.00, 0.01));
    auto lineMeshA_curr = makeOneSegmentLineMesh(Vec3d(0.00, 0.00, -0.01), Vec3d(0.00, 0.00, 0.01));
    auto lineMeshB_prev = makeOneSegmentLineMesh(Vec3d(-0.01, 0.01, 0.00), Vec3d(0.01, 0.01, 0.00));
    auto lineMeshB_curr = makeOneSegmentLineMesh(Vec3d(-0.01, -0.01, 0.00), Vec3d(0.01, -0.01, 0.00));
    intersectionTest(lineMeshA_prev, lineMeshB_prev, lineMeshA_curr, lineMeshB_curr);
}

// 2(a)
TEST(imstkLineMeshToLineMeshCCDTest, IntersectionTestAB_vertex_vertex_crossing)
{
    auto lineMeshA_prev = makeOneSegmentLineMesh(Vec3d(0.00, 0.00, -0.01), Vec3d(0.00, 0.00, 0.01));
    auto lineMeshA_curr = makeOneSegmentLineMesh(Vec3d(0.00, 0.00, -0.01), Vec3d(0.00, 0.00, 0.01));
    auto lineMeshB_prev = makeOneSegmentLineMesh(Vec3d(-0.01, 0.01, -0.01), Vec3d(0.00, 0.01, -0.01));
    auto lineMeshB_curr = makeOneSegmentLineMesh(Vec3d(-0.01, -0.01, -0.01), Vec3d(0.00, -0.01, -0.01));
    intersectionTest(lineMeshA_prev, lineMeshB_prev, lineMeshA_curr, lineMeshB_curr);
}

// 3(a)
TEST(imstkLineMeshToLineMeshCCDTest, IntersectionTestAB_edge_vertex_crossing)
{
    auto lineMeshA_prev = makeOneSegmentLineMesh(Vec3d(0.00, 0.00, -0.01), Vec3d(0.00, 0.00, 0.01));
    auto lineMeshA_curr = makeOneSegmentLineMesh(Vec3d(0.00, 0.00, -0.01), Vec3d(0.00, 0.00, 0.01));
    auto lineMeshB_prev = makeOneSegmentLineMesh(Vec3d(-0.01, 0.01, 0.00), Vec3d(0.00, 0.01, 0.00));
    auto lineMeshB_curr = makeOneSegmentLineMesh(Vec3d(-0.01, -0.01, 0.00), Vec3d(0.00, -0.01, 0.00));
    intersectionTest(lineMeshA_prev, lineMeshB_prev, lineMeshA_curr, lineMeshB_curr);
}

// --- direct overlap cases:
// 1(b)
TEST(imstkLineMeshToLineMeshCCDTest, IntersectionTestAB_edge_edge_overlap)
{
    auto lineMeshA_prev = makeOneSegmentLineMesh(Vec3d(0.00, 0.00, -0.01), Vec3d(0.00, 0.00, 0.01));
    auto lineMeshA_curr = makeOneSegmentLineMesh(Vec3d(0.00, 0.00, -0.01), Vec3d(0.00, 0.00, 0.01));
    auto lineMeshB_prev = makeOneSegmentLineMesh(Vec3d(-0.01, 0.01, 0.00), Vec3d(0.01, 0.01, 0.00));
    auto lineMeshB_curr = makeOneSegmentLineMesh(Vec3d(-0.01, 0.00, 0.00), Vec3d(0.01, 0.00, 0.00));
    intersectionTest(lineMeshA_prev, lineMeshB_prev, lineMeshA_curr, lineMeshB_curr);
}

// 2(b)
TEST(imstkLineMeshToLineMeshCCDTest, IntersectionTestAB_vertex_vertex_overlap)
{
    auto lineMeshA_prev = makeOneSegmentLineMesh(Vec3d(0.00, 0.00, -0.01), Vec3d(0.00, 0.00, 0.01));
    auto lineMeshA_curr = makeOneSegmentLineMesh(Vec3d(0.00, 0.00, -0.01), Vec3d(0.00, 0.00, 0.01));
    auto lineMeshB_prev = makeOneSegmentLineMesh(Vec3d(-0.01, 0.01, -0.01), Vec3d(0.00, 0.01, -0.01));
    auto lineMeshB_curr = makeOneSegmentLineMesh(Vec3d(-0.01, 0.00, -0.01), Vec3d(0.00, 0.00, -0.01));
    intersectionTest(lineMeshA_prev, lineMeshB_prev, lineMeshA_curr, lineMeshB_curr);
}

// 3(b)
TEST(imstkLineMeshToLineMeshCCDTest, IntersectionTestAB_edge_vertex_overlap)
{
    auto lineMeshA_prev = makeOneSegmentLineMesh(Vec3d(0.00, 0.00, -0.01), Vec3d(0.00, 0.00, 0.01));
    auto lineMeshA_curr = makeOneSegmentLineMesh(Vec3d(0.00, 0.00, -0.01), Vec3d(0.00, 0.00, 0.01));
    auto lineMeshB_prev = makeOneSegmentLineMesh(Vec3d(-0.01, 0.01, 0.00), Vec3d(0.00, 0.01, 0.00));
    auto lineMeshB_curr = makeOneSegmentLineMesh(Vec3d(-0.01, 0.00, 0.00), Vec3d(0.00, 0.00, 0.00));
    intersectionTest(lineMeshA_prev, lineMeshB_prev, lineMeshA_curr, lineMeshB_curr);
}

// Self intersection.
TEST(imstkLineMeshToLineMeshCCDTest, IntersectionTestAA_self)
{
    std::vector<Vec3d> points;
    points.push_back(Vec3d(1, 0, 1));
    points.push_back(Vec3d(1, 0, -1));
    points.push_back(Vec3d(0, 0, 0));
    points.push_back(Vec3d(2, 1, 0));
    auto lineMesh_prev = makeLineMesh(points);

    points.back().y() = -1;
    auto lineMesh_curr = makeLineMesh(points);

    // create collision
    LineMeshToLineMeshCCD m_lineMeshToLineMeshCCD;
    m_lineMeshToLineMeshCCD.updatePreviousTimestepGeometry(lineMesh_prev, lineMesh_prev);
    m_lineMeshToLineMeshCCD.setInput(lineMesh_curr, 0);
    m_lineMeshToLineMeshCCD.setInput(lineMesh_curr, 1);
    m_lineMeshToLineMeshCCD.setGenerateCD(true, true); // Generate both A and B
    m_lineMeshToLineMeshCCD.update();

    std::shared_ptr<CollisionData> colData = m_lineMeshToLineMeshCCD.getCollisionData();

    // Should be one element on side A, 1 on side B
    EXPECT_EQ(1, colData->elementsA.size());
    EXPECT_EQ(1, colData->elementsB.size());

    // m_ccdData flag should be true
    EXPECT_EQ(true, colData->elementsA[0].m_ccdData);
    EXPECT_EQ(true, colData->elementsB[0].m_ccdData);

    // That element should be a point directional element
    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsA[0].m_type);
    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsB[0].m_type);

    // Check cell types
    EXPECT_EQ(IMSTK_EDGE, colData->elementsA[0].m_element.m_CellIndexElement.cellType);
    EXPECT_EQ(IMSTK_EDGE, colData->elementsB[0].m_element.m_CellIndexElement.cellType);

    // Check id counts
    EXPECT_EQ(2, colData->elementsA[0].m_element.m_CellIndexElement.idCount);
    EXPECT_EQ(2, colData->elementsB[0].m_element.m_CellIndexElement.idCount);

    // Check cell ids
    EXPECT_EQ(0, colData->elementsA[0].m_element.m_CellIndexElement.ids[0]);
    EXPECT_EQ(1, colData->elementsA[0].m_element.m_CellIndexElement.ids[1]);
    EXPECT_EQ(2, colData->elementsB[0].m_element.m_CellIndexElement.ids[0]);
    EXPECT_EQ(3, colData->elementsB[0].m_element.m_CellIndexElement.ids[1]);
}

// No intersection.
TEST(imstkLineMeshToLineMeshCCDTest, NonIntersectionTestAB)
{
    // Create non-intersecting line mesh
    auto lineMeshA_prev = makeOneSegmentLineMesh(Vec3d(0.00, 0.00, -0.01), Vec3d(0.00, 0.00, 0.01));
    auto lineMeshA_curr = makeOneSegmentLineMesh(Vec3d(0.00, 0.00, -0.01), Vec3d(0.00, 0.00, 0.01));
    auto lineMeshB_prev = makeOneSegmentLineMesh(Vec3d(-0.01, 0.01, 0.00), Vec3d(0.01, 0.01, 0.00));
    auto lineMeshB_curr = makeOneSegmentLineMesh(Vec3d(-0.01, 0.02, 0.00), Vec3d(0.01, 0.02, 0.00));

    // Create collision data
    LineMeshToLineMeshCCD m_lineMeshToLineMeshCCD;
    m_lineMeshToLineMeshCCD.updatePreviousTimestepGeometry(lineMeshA_prev, lineMeshB_prev);
    m_lineMeshToLineMeshCCD.setInput(lineMeshA_curr, 0);
    m_lineMeshToLineMeshCCD.setInput(lineMeshB_curr, 1);
    m_lineMeshToLineMeshCCD.setGenerateCD(true, true); // Generate both A and B
    m_lineMeshToLineMeshCCD.update();
    std::shared_ptr<CollisionData> colData = m_lineMeshToLineMeshCCD.getCollisionData();

    // Should have no elements
    EXPECT_EQ(0, colData->elementsA.size());
    EXPECT_EQ(0, colData->elementsB.size());
}
