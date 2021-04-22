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
#include "imstkSurfaceMesh.h"
#include "imstkVecDataArray.h"
#include "imstkMath.h"

using namespace imstk;

namespace {

auto floatArray3 = std::make_shared<VecDataArray<float, 3>>(VecDataArray<float, 3>({
    Vec3f{0.0f, 0.1f, 0.2f},
    Vec3f{1.0f, 1.1f, 1.2f},
    Vec3f{2.0f, 2.1f, 2.2f},
    }));

auto doubleArray3 = std::make_shared<VecDataArray<double, 3>>(VecDataArray<double, 3>({
    Vec3d{0.0, 0.1, 0.2},
    Vec3d{1.0, 1.1, 1.2},
    Vec3d{2.0, 2.1, 2.2},
    Vec3d{3.0, 3.1, 3.2},
    }));

auto floatArray2 = std::make_shared<VecDataArray<float, 2>>(VecDataArray<float, 2>({
    Vec2f{0.0f, 0.1f},
    Vec2f{1.0f, 1.1f},
    Vec2f{2.0f, 2.1f},
    }));

auto doubleArray2 = std::make_shared<VecDataArray<double, 2>>(VecDataArray<double, 2>({
    Vec2d{0.0, 0.1},
    Vec2d{1.0, 1.1},
    Vec2d{2.0, 2.1},
    }));


std::unordered_map<std::string, std::shared_ptr<AbstractDataArray>> attributes = {
    {"float3", floatArray3}, {"double3", doubleArray3},
    {"float2", floatArray2}, {"double2", doubleArray2}
};

}

TEST(imstkSurfaceMeshTest, CellNormalAttributes)
{
    SurfaceMesh m;
    m.setCellAttributes(attributes);
    m.setCellNormals("double3");
    EXPECT_EQ(doubleArray3, m.getCellNormals());

    // Normals want doubles, test with floats
    m.setCellNormals("float3");
    auto normals = m.getCellNormals();
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

TEST(imstkSurfaceMeshTest, CellTangentAttributes)
{
    SurfaceMesh m;
    m.setCellAttributes(attributes);
    m.setCellTangents("double3");
    EXPECT_EQ(doubleArray3, m.getCellTangents());

    // Tangents want floats, test with doubles
    m.setCellTangents("float3");
    auto tangents = m.getCellTangents();
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

int
imstkSurfaceMeshTest(int argc, char* argv[])
{
    // Init Google Test & Mock
    ::testing::InitGoogleTest(&argc, argv);

    // Run tests with gtest
    return RUN_ALL_TESTS();
}
