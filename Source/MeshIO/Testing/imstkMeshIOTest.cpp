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

#include "imstkGeometryUtilities.h"
#include "imstkLineMesh.h"
#include "imstkMeshIO.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"

#include <gtest/gtest.h>

using namespace imstk;

TEST(imstkMeshIODeathTest, FailOnMissingFile)
{
    EXPECT_DEATH(MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "doesntexist.obj"), "doesntexist.obj doesn't exist");
}

// Test for issue https://gitlab.kitware.com/iMSTK/iMSTK/-/issues/365
TEST(imstkMeshIOTest, conversionBug)
{
    auto mesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "testing/MeshIO/bugs/membrane_model.vtk");
    ASSERT_TRUE(mesh);

    EXPECT_NO_FATAL_FAILURE(auto data = GeometryUtils::copyToVtkPolyData(mesh));
}

TEST(imstkMeshIOTest, ReadVtkTriangle)
{
    std::shared_ptr<PointSet> mesh =
        MeshIO::read(iMSTK_DATA_ROOT "testing/MeshIO/triangle.vtk");
    ASSERT_TRUE(mesh);

    // Assure we read a SurfaceMesh
    auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(mesh);
    ASSERT_TRUE(surfMesh);

    ASSERT_EQ(surfMesh->getNumVertices(), 3);
    ASSERT_EQ(surfMesh->getNumCells(), 1);
}

TEST(imstkMeshIOTest, ReadVtkLine)
{
    std::shared_ptr<PointSet> mesh =
        MeshIO::read(iMSTK_DATA_ROOT "testing/MeshIO/line.vtk");
    ASSERT_TRUE(mesh);

    // Assure we read a LineMesh
    auto lineMesh = std::dynamic_pointer_cast<LineMesh>(mesh);
    ASSERT_TRUE(lineMesh);

    ASSERT_EQ(lineMesh->getNumVertices(), 2);
    ASSERT_EQ(lineMesh->getNumCells(), 1);
}

TEST(imstkMeshIOTest, ReadVtkPoints)
{
    std::shared_ptr<PointSet> mesh =
        MeshIO::read(iMSTK_DATA_ROOT "testing/MeshIO/points.vtk");
    ASSERT_TRUE(mesh);

    ASSERT_EQ(mesh->getNumVertices(), 482);

    // Assure we did not read a LineMesh or SurfaceMesh
    auto lineMesh = std::dynamic_pointer_cast<LineMesh>(mesh);
    ASSERT_FALSE(lineMesh);
    auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(mesh);
    ASSERT_FALSE(surfMesh);
    auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(mesh);
    ASSERT_FALSE(tetMesh);
}

TEST(imstkMeshIOTest, ReadVtkTetrahedron)
{
    std::shared_ptr<PointSet> mesh =
        MeshIO::read(iMSTK_DATA_ROOT "testing/MeshIO/tet.vtk");
    ASSERT_TRUE(mesh);

    // Assure we read a TetrahedralMesh
    auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(mesh);
    ASSERT_TRUE(tetMesh);

    ASSERT_EQ(tetMesh->getNumVertices(), 4);
    ASSERT_EQ(tetMesh->getNumCells(), 1);
}