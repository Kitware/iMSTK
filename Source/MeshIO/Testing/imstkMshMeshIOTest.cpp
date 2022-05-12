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

#include <gtest/gtest.h>

using namespace imstk;

TEST(imstkMshMeshIOTest, ReadMsh_Ascii_LineMesh)
{
    std::shared_ptr<PointSet> mesh =
        MshMeshIO::read(iMSTK_DATA_ROOT "testing/MeshIO/lineAscii.msh");
    ASSERT_TRUE(mesh);

    // Assure we read a LineMesh
    auto tetMesh = std::dynamic_pointer_cast<LineMesh>(mesh);
    ASSERT_TRUE(tetMesh);
}
TEST(imstkMshMeshIOTest, ReadMsh_Binary_LineMesh)
{
    std::shared_ptr<PointSet> mesh =
        MshMeshIO::read(iMSTK_DATA_ROOT "testing/MeshIO/lineBinary.msh");
    ASSERT_TRUE(mesh);

    // Assure we read a LineMesh
    auto tetMesh = std::dynamic_pointer_cast<LineMesh>(mesh);
    ASSERT_TRUE(tetMesh);
}

TEST(imstkMshMeshIOTest, ReadMsh_Ascii_SurfaceMesh)
{
    std::shared_ptr<PointSet> mesh =
        MshMeshIO::read(iMSTK_DATA_ROOT "testing/MeshIO/triangleAscii.msh");
    ASSERT_TRUE(mesh);

    // Assure we read a SurfaceMesh
    auto tetMesh = std::dynamic_pointer_cast<SurfaceMesh>(mesh);
    ASSERT_TRUE(tetMesh);
}
TEST(imstkMshMeshIOTest, ReadMsh_Binary_SurfaceMesh)
{
    std::shared_ptr<PointSet> mesh =
        MshMeshIO::read(iMSTK_DATA_ROOT "testing/MeshIO/triangleBinary.msh");
    ASSERT_TRUE(mesh);

    // Assure we read a SurfaceMesh
    auto tetMesh = std::dynamic_pointer_cast<SurfaceMesh>(mesh);
    ASSERT_TRUE(tetMesh);
}

TEST(imstkMshMeshIOTest, ReadMsh_Ascii_TetrahedralMesh)
{
    std::shared_ptr<PointSet> mesh =
        MshMeshIO::read(iMSTK_DATA_ROOT "testing/MeshIO/tetAscii.msh");
    ASSERT_TRUE(mesh);

    // Assure we read a SurfaceMesh
    auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(mesh);
    ASSERT_TRUE(tetMesh);
}
TEST(imstkMshMeshIOTest, ReadMsh_Binary_TetrahedralMesh)
{
    std::shared_ptr<PointSet> mesh =
        MshMeshIO::read(iMSTK_DATA_ROOT "testing/MeshIO/tetBinary.msh");
    ASSERT_TRUE(mesh);

    // Assure we read a SurfaceMesh
    auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(mesh);
    ASSERT_TRUE(tetMesh);
}

TEST(imstkMshMeshIOTest, ReadMsh_Human)
{
    std::shared_ptr<PointSet> mesh =
        MshMeshIO::read(iMSTK_DATA_ROOT "human/human.msh");
    ASSERT_TRUE(mesh);

    // Assure we read a TetrahedralMesh
    auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(mesh);
    ASSERT_TRUE(tetMesh);
}