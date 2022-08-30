/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkAppendMesh.h"
#include "imstkGeometryUtilities.h"
#include "imstkSurfaceMesh.h"

#include <gtest/gtest.h>

using namespace imstk;

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

TEST(AppendMeshTest, Filter)
{
    // Create first surface mesh
    auto mesh1 = makeRect();

    // Create second surface mesh
    auto mesh2 = makeRect();

    // Append meshes
    AppendMesh append;
    append.addInputMesh(mesh1);
    append.addInputMesh(mesh2);
    append.update();

    auto outMesh = append.getOutputMesh();

    // Test equivalence
    EXPECT_EQ(outMesh->getNumVertices(), mesh1->getNumVertices() + mesh2->getNumVertices());
    EXPECT_EQ(outMesh->getNumCells(), mesh1->getNumCells() + mesh2->getNumCells());
}