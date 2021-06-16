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

#include "imstkOrientedBox.h"
#include "imstkGeometryUtilities.h"
#include "imstkMath.h"
#include "imstkPointSet.h"
#include "imstkSurfaceMesh.h"
#include "imstkVecDataArray.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
using namespace imstk;

namespace
{
auto floatArray3 = std::make_shared<VecDataArray<float, 3>>(VecDataArray<float, 3>({
        Vec3f{ 0.0f, 0.1f, 0.2f },
        Vec3f{ 1.0f, 1.1f, 1.2f },
        Vec3f{ 2.0f, 2.1f, 2.2f },
    }));

auto doubleArray3 = std::make_shared<VecDataArray<double, 3>>(VecDataArray<double, 3>({
        Vec3d{ 0.0, 0.1, 0.2 },
        Vec3d{ 1.0, 1.1, 1.2 },
        Vec3d{ 2.0, 2.1, 2.2 },
        Vec3d{ 3.0, 3.1, 3.2 },
    }));

auto floatArray2 = std::make_shared<VecDataArray<float, 2>>(VecDataArray<float, 2>({
        Vec2f{ 0.0f, 0.1f },
        Vec2f{ 1.0f, 1.1f },
        Vec2f{ 2.0f, 2.1f },
    }));

auto doubleArray2 = std::make_shared<VecDataArray<double, 2>>(VecDataArray<double, 2>({
        Vec2d{ 0.0, 0.1 },
        Vec2d{ 1.0, 1.1 },
        Vec2d{ 2.0, 2.1 },
    }));

std::unordered_map<std::string, std::shared_ptr<AbstractDataArray>> attributes = {
    { "float3", floatArray3 }, { "double3", doubleArray3 },
    { "float2", floatArray2 }, { "double2", doubleArray2 }
};

// A Rectangle with the vertices along the long sides
// 0****1
// *   **
// *  * *
// * *  *
// **   *
// 2****3
// *   **
// 4    5
// ...
std::shared_ptr<SurfaceMesh>
makeRect()
{
    imstk::VecDataArray<double, 3> points;
    auto                           scalars = std::make_shared<imstk::DataArray<float>>();

    for (int i = 0; i < 6; ++i)
    {
        points.push_back({ 0, 0, static_cast<double>(i) });
        scalars->push_back(i);
        points.push_back({ 1, 0, static_cast<double>(i) });
        scalars->push_back(i);
    }

    auto mesh = std::make_shared<imstk::SurfaceMesh>();

    imstk::VecDataArray<int, 3> tris;
    for (int i = 0; i < 5; ++i)
    {
        int j = i * 2;
        tris.push_back({ j + 2, j + 1, j });
        tris.push_back({ j + 3, j + 1, j + 2 });
    }

    mesh->initialize(std::make_shared<VecDataArray<double, 3>>(points), std::make_shared<VecDataArray<int, 3>>(tris));
    mesh->setVertexAttribute("scalars", scalars);
    mesh->setVertexScalars("scalars");

    return mesh;
}
}

TEST(imstkSurfaceMeshTest, CellNormalAttributes)
{
    SurfaceMesh surfMesh;
    surfMesh.setCellAttributes(attributes);
    surfMesh.setCellNormals("double3");
    EXPECT_EQ(doubleArray3, surfMesh.getCellNormals());

    // Normals want doubles, test with floats
    surfMesh.setCellNormals("float3");
    auto normals = surfMesh.getCellNormals();
    ASSERT_NE(nullptr, normals);
    EXPECT_NE(floatArray3->getVoidPointer(), normals->getVoidPointer());
    EXPECT_EQ(3, normals->size());
    for (int i = 0; i < normals->size(); ++i)
    {
        EXPECT_TRUE((*floatArray3)[i].cast<double>().isApprox((*normals)[i]));
    }

    // This could work we'd need to make the DataArray a little bit more standards compliant
    // EXPECT_THAT(*normals, ElementsAreArray(doubleArray3->begin(), doubleArray3->end()));
}

TEST(imstkSurfaceMeshTest, VertexNeighborVertices)
{
    using testing::UnorderedElementsAre;

    auto mesh = makeRect();

    mesh->computeVertexNeighborVertices();

    auto neighbors = mesh->getVertexNeighborVertices();
    EXPECT_THAT(neighbors[0], UnorderedElementsAre(1, 2));
    EXPECT_THAT(neighbors[1], UnorderedElementsAre(0, 2, 3));
    EXPECT_THAT(neighbors[3], UnorderedElementsAre(1, 2, 4, 5));
}

TEST(imstkSurfaceMeshTest, VertexTriangleNeigbors)
{
    using testing::UnorderedElementsAre;

    auto mesh = makeRect();

    mesh->computeVertexNeighborTriangles();

    auto neighbors = mesh->getVertexNeighborTriangles();
    EXPECT_THAT(neighbors[0], UnorderedElementsAre(0));
    EXPECT_THAT(neighbors[1], UnorderedElementsAre(0, 1));
    EXPECT_THAT(neighbors[3], UnorderedElementsAre(1, 2, 3));
}

TEST(imstkSurfaceMeshTest, CellTangentAttributes)
{
    SurfaceMesh surfMesh;
    surfMesh.setCellAttributes(attributes);
    surfMesh.setCellTangents("double3");
    EXPECT_EQ(doubleArray3, surfMesh.getCellTangents());

    // Tangents want floats, test with doubles
    surfMesh.setCellTangents("float3");
    auto tangents = surfMesh.getCellTangents();
    ASSERT_NE(nullptr, tangents);
    EXPECT_NE(floatArray3->getVoidPointer(), tangents->getVoidPointer());
    EXPECT_EQ(3, tangents->size());
    for (int i = 0; i < tangents->size(); ++i)
    {
        EXPECT_TRUE((*floatArray3)[i].cast<double>().isApprox((*tangents)[i]));
    }
}

TEST(imstkSurfaceMeshTest, ComputeTriangleNormals)
{

    // This is counter clockwise, when looking down on y, so normal should be directly up
    // opengl coordinate system with -z going "out" from identity view
    auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(3);
    (*verticesPtr)[0] = Vec3d(0.5, 0.0, -0.5);
    (*verticesPtr)[1] = Vec3d(-0.5, 0.0, -0.5);
    (*verticesPtr)[2] = Vec3d(0.0, 0.0, 0.5);
    {
        SurfaceMesh surfMesh;
        auto indicesPtr = std::make_shared<VecDataArray<int, 3>>(1);
        (*indicesPtr)[0] = Vec3i(0, 1, 2);
        surfMesh.initialize(verticesPtr, indicesPtr);

        surfMesh.computeTrianglesNormals();
        auto normalsPtr = surfMesh.getCellNormals();

        EXPECT_NE(nullptr, normalsPtr);
        EXPECT_EQ(1, normalsPtr->size());
        EXPECT_TRUE(Vec3d(0.0, 1.0, 0.0).isApprox((*normalsPtr)[0]));
    }
    {    
        SurfaceMesh surfMesh;
        auto indicesPtr = std::make_shared<VecDataArray<int, 3>>(1);
        (*indicesPtr)[0] = Vec3i(2, 1, 0);
        surfMesh.initialize(verticesPtr, indicesPtr);

        surfMesh.computeTrianglesNormals();
        auto normalsPtr = surfMesh.getCellNormals();

        EXPECT_NE(nullptr, normalsPtr);
        EXPECT_EQ(1, normalsPtr->size());
        EXPECT_TRUE(Vec3d(0.0, -1.0, 0.0).isApprox((*normalsPtr)[0]));
    }

}

TEST(imstkSurfaceMeshTest, ComputeVertexNormals)
{
    //
    //   /|\
    //  / | \
    // //   \\
    //
    // Tests two triangles that share an edge
    auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(4);
    (*verticesPtr)[0] = Vec3d(0.0, 0.0, -1.0);
    (*verticesPtr)[1] = Vec3d(0.0, 0.0, 1.0);
    (*verticesPtr)[2] = Vec3d(1.0, -1.0, 0.0);
    (*verticesPtr)[3] = Vec3d(-1.0, -1.0, 0.0);

    auto indicesPtr = std::make_shared<VecDataArray<int, 3>>(2);
    (*indicesPtr)[0] = Vec3i(0, 1, 2);
    (*indicesPtr)[1] = Vec3i(0, 3, 1);

    SurfaceMesh surfMesh;
    surfMesh.initialize(verticesPtr, indicesPtr);

    // Should make 45 degrees 1, 1 edge
    surfMesh.computeVertexNormals();
    auto normalsPtr = surfMesh.getVertexNormals();

    const Vec3d results1 = Vec3d(1.0, 1.0, 0.0).normalized();
    const Vec3d results2 = Vec3d(-1.0, 1.0, 0.0).normalized();

    // Check the endpoint normals (these are summed to the face)
    EXPECT_NEAR(results1[0], (*normalsPtr)[2][0], 1e-8);
    EXPECT_NEAR(results1[1], (*normalsPtr)[2][1], 1e-8);
    EXPECT_NEAR(results1[2], (*normalsPtr)[2][2], 1e-8);

    EXPECT_NEAR(results2[0], (*normalsPtr)[3][0], 1e-8);
    EXPECT_NEAR(results2[1], (*normalsPtr)[3][1], 1e-8);
    EXPECT_NEAR(results2[2], (*normalsPtr)[3][2], 1e-8);

    // Check the shared vertex normals which should point straight up
    EXPECT_EQ(Vec3d(0.0, 1.0, 0.0), (*normalsPtr)[0]);
    EXPECT_EQ(Vec3d(0.0, 1.0, 0.0), (*normalsPtr)[1]);
}

TEST(imstkSurfaceMeshTest, GetVolume)
{
    std::shared_ptr<SurfaceMesh> cubeSurfMesh =
        GeometryUtils::toSurfaceMesh(std::make_shared<OrientedBox>());
    EXPECT_DOUBLE_EQ(1.0, cubeSurfMesh->getVolume());

    cubeSurfMesh->scale(2);
    cubeSurfMesh->updatePostTransformData();

    EXPECT_DOUBLE_EQ(8.0, cubeSurfMesh->getVolume());
}
