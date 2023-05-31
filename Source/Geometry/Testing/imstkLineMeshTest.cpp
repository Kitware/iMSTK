/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkLineMesh.h"

#include <gtest/gtest.h>

using namespace imstk;

TEST(imstkLineMeshTest, VertexNeighborVertices)
{
    auto                    lineMesh = std::make_shared<LineMesh>();
    VecDataArray<double, 3> vertices = { Vec3d(-0.5, 0.0, 0.0), Vec3d(0.0, 0.0, 0.0), Vec3d(0.5, 0.0, 0.0) };
    VecDataArray<int, 2>    indices  = { Vec2i(0, 1), Vec2i(1, 2) };
    lineMesh->initialize(
        std::make_shared<VecDataArray<double, 3>>(vertices),
        std::make_shared<VecDataArray<int, 2>>(indices));

    lineMesh->computeVertexNeighbors();

    const std::vector<std::unordered_set<int>>& map = lineMesh->getVertexNeighbors();

    EXPECT_EQ(3, map.size());                 // Should contain three vertices

    EXPECT_EQ(1, map[0].size());              // Has one neighbor
    EXPECT_EQ(2, map[1].size());              // Has two neighbors
    EXPECT_EQ(1, map[2].size());              // Has one neighbor

    EXPECT_EQ(1, *map[0].begin());            // v0 should have v1 as neighbor
    EXPECT_EQ(0, *map[1].begin());            // v1 should have v0 & v2 as neighbor
    EXPECT_EQ(2, *std::next(map[1].begin())); // v1 should have v0 & v2 as neighbor
    EXPECT_EQ(1, *map[2].begin());            // v2 should have v1 as neighbor
}

TEST(imstkLineMeshTest, VertexLineNeigbors)
{
    auto                    lineMesh = std::make_shared<LineMesh>();
    VecDataArray<double, 3> vertices = { Vec3d(-0.5, 0.0, 0.0), Vec3d(0.5, 0.0, 0.0) };
    VecDataArray<int, 2>    indices  = { Vec2i(0, 1) };
    lineMesh->initialize(
        std::make_shared<VecDataArray<double, 3>>(vertices),
        std::make_shared<VecDataArray<int, 2>>(indices));

    lineMesh->computeVertexToCellMap();
    const std::vector<std::unordered_set<int>>& map = lineMesh->getVertexToCellMap();

    EXPECT_EQ(2, map.size());      // Should contain two vertices
    EXPECT_EQ(1, map[0].size());   // Vertex 0 should map to one line
    EXPECT_EQ(0, *map[0].begin()); // It should map to a line whose cell index is 0
    EXPECT_EQ(1, map[1].size());   // Vertex 1 should map to one line
    EXPECT_EQ(0, *map[1].begin()); // It should map to a line whose cell index is 0
}

TEST(imstkLineMeshTest, ComputeWorldPosition)
{
    auto                    lineMesh = std::make_shared<LineMesh>();
    VecDataArray<double, 3> vertices = { Vec3d(-0.5, 0.0, 0.0), Vec3d(0.5, 0.0, 0.0) };
    VecDataArray<int, 2>    indices = { Vec2i(0, 1) };
    lineMesh->initialize(
        std::make_shared<VecDataArray<double, 3>>(vertices),
        std::make_shared<VecDataArray<int, 2>>(indices));

    Vec2d baryPt = Vec2d::Zero();
    Vec3d pos = Vec3d::Zero();

    // Test cell 0 node 0
    baryPt = Vec2d(1.0, 0.0);
    pos = lineMesh->computeWorldPosition(0, baryPt);
    EXPECT_EQ(pos, Vec3d(-0.5, 0.0, 0.0));

    // Test cell 0 node 1
    baryPt = Vec2d(0.0, 1.0);
    pos = lineMesh->computeWorldPosition(0, baryPt);
    EXPECT_EQ(pos, Vec3d(0.5, 0.0, 0.0));

    // Test cell 0 halfway
    baryPt = Vec2d(0.5, 0.5);
    pos = lineMesh->computeWorldPosition(0, baryPt);
    EXPECT_EQ(pos, Vec3d(0.0, 0.0, 0.0));

    // Test cell 0 quarterWay
    baryPt = Vec2d(0.25, 0.75);
    pos = lineMesh->computeWorldPosition(0, baryPt);
    EXPECT_EQ(pos, Vec3d(0.25, 0.0, 0.0));
}
