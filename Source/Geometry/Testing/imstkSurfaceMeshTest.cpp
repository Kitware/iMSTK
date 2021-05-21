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
}

///
/// \brief TODO
///
class imstkSurfaceMeshTest : public ::testing::Test
{
protected:
    SurfaceMesh m_surfMesh;
};

TEST_F(imstkSurfaceMeshTest, CellNormalAttributes)
{
    m_surfMesh.setCellAttributes(attributes);
    m_surfMesh.setCellNormals("double3");
    EXPECT_EQ(doubleArray3, m_surfMesh.getCellNormals());

    // Normals want doubles, test with floats
    m_surfMesh.setCellNormals("float3");
    auto normals = m_surfMesh.getCellNormals();
    ASSERT_NE(nullptr, normals);
    EXPECT_NE(floatArray3->getVoidPointer(), normals->getVoidPointer());
    EXPECT_EQ(3, normals->size());
    for (int i = 0; i < normals->size(); ++i)
    {
        EXPECT_TRUE((*floatArray3)[i].cast<double>().isApprox((*normals)[i]));
    }

    // This could work we'd need to make the DataArray a little bit more standards compliant
    // EXPECT_THAT(*normals, ElementsAreArray(doubleArray3->begin(), doubleArray3->end()));
    // HS 2021-apr-04 Death tests don't work with the current infrastructure
    //ASSERT_DEATH(p.setVertexNormals("float2"), ".*");
}

TEST_F(imstkSurfaceMeshTest, CellTangentAttributes)
{
    m_surfMesh.setCellAttributes(attributes);
    m_surfMesh.setCellTangents("double3");
    EXPECT_EQ(doubleArray3, m_surfMesh.getCellTangents());

    // Tangents want floats, test with doubles
    m_surfMesh.setCellTangents("float3");
    auto tangents = m_surfMesh.getCellTangents();
    ASSERT_NE(nullptr, tangents);
    EXPECT_NE(floatArray3->getVoidPointer(), tangents->getVoidPointer());
    EXPECT_EQ(3, tangents->size());
    for (int i = 0; i < tangents->size(); ++i)
    {
        EXPECT_TRUE((*floatArray3)[i].cast<double>().isApprox((*tangents)[i]));
    }

    // HS 2021-apr-04 Death tests don't work with the current infrastructure
    //ASSERT_DEATH(p.setVertexTangents("float2"), ".*");
}

///
/// \brief Tests the correct computation of face normals
///
TEST_F(imstkSurfaceMeshTest, ComputeTriangleNormals)
{
    // This is counter clockwise, when looking down on y, so normal should be directly up
    // opengl coordinate system with -z going "out" from identity view
    auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(3);
    (*verticesPtr)[0] = Vec3d(0.5, 0.0, -0.5);
    (*verticesPtr)[1] = Vec3d(-0.5, 0.0, -0.5);
    (*verticesPtr)[2] = Vec3d(0.0, 0.0, 0.5);

    auto indicesPtr = std::make_shared<VecDataArray<int, 3>>(1);
    (*indicesPtr)[0] = Vec3i(0, 1, 2);
    m_surfMesh.initialize(verticesPtr, indicesPtr);

    m_surfMesh.computeTrianglesNormals();
    auto normalsPtr = m_surfMesh.getCellNormals();

    EXPECT_NE(nullptr, normalsPtr);
    EXPECT_EQ(1, normalsPtr->size());
    EXPECT_EQ(Vec3d(0.0, 1.0, 0.0), (*normalsPtr)[0]);
}

TEST_F(imstkSurfaceMeshTest, ComputeVertexNormals)
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
    m_surfMesh.initialize(verticesPtr, indicesPtr);

    // Should make 45 degrees 1, 1 edge
    m_surfMesh.computeVertexNormals();
    auto normalsPtr = m_surfMesh.getVertexNormals();

    const Vec3d results1 = Vec3d(1.0, 1.0, 0.0).normalized();
    const Vec3d results2 = Vec3d(-1.0, 1.0, 0.0).normalized();

    // Check the endpoint normals (these are summed to the face)
    EXPECT_NEAR(results1[0], (*normalsPtr)[2][0], 0.00000001);
    EXPECT_NEAR(results1[1], (*normalsPtr)[2][1], 0.00000001);
    EXPECT_NEAR(results1[2], (*normalsPtr)[2][2], 0.00000001);

    EXPECT_NEAR(results2[0], (*normalsPtr)[3][0], 0.00000001);
    EXPECT_NEAR(results2[1], (*normalsPtr)[3][1], 0.00000001);
    EXPECT_NEAR(results2[2], (*normalsPtr)[3][2], 0.00000001);

    // Check the shared vertex normals which should point straight up
    EXPECT_EQ(Vec3d(0.0, 1.0, 0.0), (*normalsPtr)[0]);
    EXPECT_EQ(Vec3d(0.0, 1.0, 0.0), (*normalsPtr)[1]);
}

///
/// \brief Tests the correct computation of volume
///
TEST_F(imstkSurfaceMeshTest, GetVolume)
{
    std::shared_ptr<SurfaceMesh> cubeSurfMesh =
        GeometryUtils::toSurfaceMesh(std::make_shared<OrientedBox>());
    EXPECT_NEAR(1.0, cubeSurfMesh->getVolume(), 0.0000000000001);
}

int
imstkSurfaceMeshTest(int argc, char* argv[])
{
    // Init Google Test & Mock
    ::testing::InitGoogleTest(&argc, argv);

    // Run tests with gtest
    return RUN_ALL_TESTS();
}
