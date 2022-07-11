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

#include "imstkHexahedralMesh.h"
#include "imstkLineMesh.h"
#include "imstkMshMeshIO.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVecDataArray.h"

#include <gtest/gtest.h>

using namespace imstk;

TEST(imstkMshMeshIOTest, ReadMsh_Ascii_LineMesh)
{
    std::shared_ptr<PointSet> mesh =
        MshMeshIO::read(iMSTK_DATA_ROOT "testing/MeshIO/lineAscii.msh");
    ASSERT_TRUE(mesh);

    // Assure we read a LineMesh
    auto lineMesh = std::dynamic_pointer_cast<LineMesh>(mesh);
    ASSERT_TRUE(lineMesh);

    // Check the sizes
    ASSERT_EQ(2, lineMesh->getVertexPositions()->size());
    ASSERT_EQ(1, lineMesh->getCells()->size());

    // Check contents
    const VecDataArray<double, 3>& vertices = *lineMesh->getVertexPositions();
    ASSERT_EQ(Vec3d(-0.5, 0.0, 0.0), vertices[0]);
    ASSERT_EQ(Vec3d(0.5, 0.0, 0.0), vertices[1]);
    const VecDataArray<int, 2>& indices = *lineMesh->getCells();
    ASSERT_EQ(0, indices[0][0]);
    ASSERT_EQ(1, indices[0][1]);
}
TEST(imstkMshMeshIOTest, ReadMsh_Binary_LineMesh)
{
    std::shared_ptr<PointSet> mesh =
        MshMeshIO::read(iMSTK_DATA_ROOT "testing/MeshIO/lineBinary.msh");
    ASSERT_TRUE(mesh);

    // Assure we read a LineMesh
    auto lineMesh = std::dynamic_pointer_cast<LineMesh>(mesh);
    ASSERT_TRUE(lineMesh);

    // Check the sizes
    ASSERT_EQ(2, lineMesh->getVertexPositions()->size());
    ASSERT_EQ(1, lineMesh->getCells()->size());

    // Check contents
    const VecDataArray<double, 3>& vertices = *lineMesh->getVertexPositions();
    ASSERT_EQ(Vec3d(-0.5, 0.0, 0.0), vertices[0]);
    ASSERT_EQ(Vec3d(0.5, 0.0, 0.0), vertices[1]);
    const VecDataArray<int, 2>& indices = *lineMesh->getCells();
    ASSERT_EQ(0, indices[0][0]);
    ASSERT_EQ(1, indices[0][1]);
}

TEST(imstkMshMeshIOTest, ReadMsh_Ascii_SurfaceMesh)
{
    std::shared_ptr<PointSet> mesh =
        MshMeshIO::read(iMSTK_DATA_ROOT "testing/MeshIO/triangleAscii.msh");
    ASSERT_TRUE(mesh);

    // Assure we read a SurfaceMesh
    auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(mesh);
    ASSERT_TRUE(surfMesh);

    // Check the sizes
    ASSERT_EQ(3, surfMesh->getVertexPositions()->size());
    ASSERT_EQ(1, surfMesh->getCells()->size());

    // Check contents
    const VecDataArray<double, 3>& vertices = *surfMesh->getVertexPositions();
    ASSERT_EQ(Vec3d(-0.5, 0.0, 0.0), vertices[0]);
    ASSERT_EQ(Vec3d(0.5, 0.0, 0.0), vertices[1]);
    ASSERT_EQ(Vec3d(0.0, 0.5, 0.0), vertices[2]);
    const VecDataArray<int, 3>& indices = *surfMesh->getCells();
    ASSERT_EQ(0, indices[0][0]);
    ASSERT_EQ(1, indices[0][1]);
    ASSERT_EQ(2, indices[0][2]);
}
TEST(imstkMshMeshIOTest, ReadMsh_Binary_SurfaceMesh)
{
    std::shared_ptr<PointSet> mesh =
        MshMeshIO::read(iMSTK_DATA_ROOT "testing/MeshIO/triangleBinary.msh");
    ASSERT_TRUE(mesh);

    // Assure we read a SurfaceMesh
    auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(mesh);
    ASSERT_TRUE(surfMesh);

    // Check the sizes
    ASSERT_EQ(3, surfMesh->getVertexPositions()->size());
    ASSERT_EQ(1, surfMesh->getCells()->size());

    // Check contents
    const VecDataArray<double, 3>& vertices = *surfMesh->getVertexPositions();
    ASSERT_EQ(Vec3d(-0.5, 0.0, 0.0), vertices[0]);
    ASSERT_EQ(Vec3d(0.5, 0.0, 0.0), vertices[1]);
    ASSERT_EQ(Vec3d(0.0, 0.5, 0.0), vertices[2]);
    const VecDataArray<int, 3>& indices = *surfMesh->getCells();
    ASSERT_EQ(0, indices[0][0]);
    ASSERT_EQ(1, indices[0][1]);
    ASSERT_EQ(2, indices[0][2]);
}

TEST(imstkMshMeshIOTest, ReadMsh_Ascii_TetrahedralMesh)
{
    std::shared_ptr<PointSet> mesh =
        MshMeshIO::read(iMSTK_DATA_ROOT "testing/MeshIO/tetAscii.msh");
    ASSERT_TRUE(mesh);

    // Assure we read a TetrahedralMesh
    auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(mesh);
    ASSERT_TRUE(tetMesh);

    // Check the sizes
    ASSERT_EQ(4, tetMesh->getVertexPositions()->size());
    ASSERT_EQ(1, tetMesh->getCells()->size());

    // Check contents
    const VecDataArray<double, 3>& vertices = *tetMesh->getVertexPositions();
    ASSERT_EQ(Vec3d(0.0, 0.707107, 1.0), vertices[0]);
    ASSERT_EQ(Vec3d(1.0, -0.707107, 0.0), vertices[1]);
    ASSERT_EQ(Vec3d(-1.0, -0.707107, 0.0), vertices[2]);
    ASSERT_EQ(Vec3d(0.0, 0.707107, -1.0), vertices[3]);
    const VecDataArray<int, 4>& indices = *tetMesh->getCells();
    ASSERT_EQ(0, indices[0][0]);
    ASSERT_EQ(1, indices[0][1]);
    ASSERT_EQ(2, indices[0][2]);
    ASSERT_EQ(3, indices[0][3]);
}
TEST(imstkMshMeshIOTest, ReadMsh_Binary_TetrahedralMesh)
{
    std::shared_ptr<PointSet> mesh =
        MshMeshIO::read(iMSTK_DATA_ROOT "testing/MeshIO/tetBinary.msh");
    ASSERT_TRUE(mesh);

    // Assure we read a TetrahedralMesh
    auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(mesh);
    ASSERT_TRUE(tetMesh);

    // Check the sizes
    ASSERT_EQ(tetMesh->getVertexPositions()->size(), 4);
    ASSERT_EQ(tetMesh->getCells()->size(), 1);
    // Check contents
    const VecDataArray<double, 3>& vertices = *tetMesh->getVertexPositions();
    ASSERT_EQ(Vec3d(0.0, 0.707107, 1.0), vertices[0]);
    ASSERT_EQ(Vec3d(1.0, -0.707107, 0.0), vertices[1]);
    ASSERT_EQ(Vec3d(-1.0, -0.707107, 0.0), vertices[2]);
    ASSERT_EQ(Vec3d(0.0, 0.707107, -1.0), vertices[3]);
    const VecDataArray<int, 4>& indices = *tetMesh->getCells();
    ASSERT_EQ(0, indices[0][0]);
    ASSERT_EQ(1, indices[0][1]);
    ASSERT_EQ(2, indices[0][2]);
    ASSERT_EQ(3, indices[0][3]);
}

TEST(imstkMshMeshIOTest, ReadMsh_Ascii_HexahedralMesh)
{
    std::shared_ptr<PointSet> mesh =
        MshMeshIO::read(iMSTK_DATA_ROOT "testing/MeshIO/hexAscii.msh");
    ASSERT_TRUE(mesh);

    // Assure we read a HexahedralMesh
    auto hexMesh = std::dynamic_pointer_cast<HexahedralMesh>(mesh);
    ASSERT_TRUE(hexMesh);

    // Check the sizes
    ASSERT_EQ(8, hexMesh->getVertexPositions()->size());
    ASSERT_EQ(1, hexMesh->getCells()->size());

    // Check contents
    const VecDataArray<double, 3>& vertices = *hexMesh->getVertexPositions();
    ASSERT_EQ(Vec3d(-0.5, -0.5, 0.5), vertices[0]);
    ASSERT_EQ(Vec3d(0.5, -0.5, 0.5), vertices[1]);
    ASSERT_EQ(Vec3d(0.5, -0.5, -0.5), vertices[2]);
    ASSERT_EQ(Vec3d(-0.5, -0.5, -0.5), vertices[3]);
    ASSERT_EQ(Vec3d(-0.5, 0.5, 0.5), vertices[4]);
    ASSERT_EQ(Vec3d(0.5, 0.5, 0.5), vertices[5]);
    ASSERT_EQ(Vec3d(0.5, 0.5, -0.5), vertices[6]);
    ASSERT_EQ(Vec3d(-0.5, 0.5, -0.5), vertices[7]);
    const VecDataArray<int, 8>& indices = *hexMesh->getCells();
    ASSERT_EQ(0, indices[0][0]);
    ASSERT_EQ(1, indices[0][1]);
    ASSERT_EQ(2, indices[0][2]);
    ASSERT_EQ(3, indices[0][3]);
    ASSERT_EQ(4, indices[0][4]);
    ASSERT_EQ(5, indices[0][5]);
    ASSERT_EQ(6, indices[0][6]);
    ASSERT_EQ(7, indices[0][7]);
}
TEST(imstkMshMeshIOTest, ReadMsh_Binary_HexahedralMesh)
{
    std::shared_ptr<PointSet> mesh =
        MshMeshIO::read(iMSTK_DATA_ROOT "testing/MeshIO/hexBinary.msh");
    ASSERT_TRUE(mesh);

    // Assure we read a HexahedralMesh
    auto hexMesh = std::dynamic_pointer_cast<HexahedralMesh>(mesh);
    ASSERT_TRUE(hexMesh);

    // Check contents
    const VecDataArray<double, 3>& vertices = *hexMesh->getVertexPositions();
    ASSERT_EQ(Vec3d(-0.5, -0.5, 0.5), vertices[0]);
    ASSERT_EQ(Vec3d(0.5, -0.5, 0.5), vertices[1]);
    ASSERT_EQ(Vec3d(0.5, -0.5, -0.5), vertices[2]);
    ASSERT_EQ(Vec3d(-0.5, -0.5, -0.5), vertices[3]);
    ASSERT_EQ(Vec3d(-0.5, 0.5, 0.5), vertices[4]);
    ASSERT_EQ(Vec3d(0.5, 0.5, 0.5), vertices[5]);
    ASSERT_EQ(Vec3d(0.5, 0.5, -0.5), vertices[6]);
    ASSERT_EQ(Vec3d(-0.5, 0.5, -0.5), vertices[7]);
    const VecDataArray<int, 8>& indices = *hexMesh->getCells();
    ASSERT_EQ(0, indices[0][0]);
    ASSERT_EQ(1, indices[0][1]);
    ASSERT_EQ(2, indices[0][2]);
    ASSERT_EQ(3, indices[0][3]);
    ASSERT_EQ(4, indices[0][4]);
    ASSERT_EQ(5, indices[0][5]);
    ASSERT_EQ(6, indices[0][6]);
    ASSERT_EQ(7, indices[0][7]);
}

TEST(imstkMshMeshIOTest, ReadMsh_Human)
{
    std::shared_ptr<PointSet> mesh =
        MshMeshIO::read(iMSTK_DATA_ROOT "human/human.msh");
    ASSERT_TRUE(mesh);

    // Assure we read a TetrahedralMesh
    auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(mesh);
    ASSERT_TRUE(tetMesh);

    // Check the sizes
    ASSERT_EQ(tetMesh->getVertexPositions()->size(), 394);
    ASSERT_EQ(tetMesh->getCells()->size(), 1184);
}