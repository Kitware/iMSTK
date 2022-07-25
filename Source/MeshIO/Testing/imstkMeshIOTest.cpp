/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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