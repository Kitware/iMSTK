/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc.

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
