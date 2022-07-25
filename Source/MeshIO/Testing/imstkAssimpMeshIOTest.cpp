/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkAssimpMeshIO.h"
#include "imstkLineMesh.h"
#include "imstkSurfaceMesh.h"

#include <gtest/gtest.h>

using namespace imstk;

TEST(imstkAssimpMeshIOTest, ReadObjTriangle)
{
    std::shared_ptr<PointSet> mesh =
        AssimpMeshIO::readMeshData(iMSTK_DATA_ROOT "testing/MeshIO/triangle.obj");
    ASSERT_TRUE(mesh);

    // Assure we read a SurfaceMesh
    auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(mesh);
    ASSERT_TRUE(surfMesh);
}

TEST(imstkAssimpMeshIOTest, ReadObjLine)
{
    std::shared_ptr<PointSet> mesh =
        AssimpMeshIO::readMeshData(iMSTK_DATA_ROOT "testing/MeshIO/line.obj");
    ASSERT_TRUE(mesh);

    // Assure we read a LineMesh
    auto lineMesh = std::dynamic_pointer_cast<LineMesh>(mesh);
    ASSERT_TRUE(lineMesh);
}