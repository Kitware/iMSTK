/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkRenderTest.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVecDataArray.h"

TEST_F(RenderTest, TetrahedralMesh)
{
    auto tetMesh = std::make_shared<TetrahedralMesh>();
    geom = tetMesh;

    VecDataArray<double, 3> vertices(4);
    vertices[0] = Vec3d(-0.5, 0.0, -0.5);
    vertices[1] = Vec3d(0.5, 0.0, -0.5);
    vertices[2] = Vec3d(0.0, 0.0, 0.75);
    vertices[3] = Vec3d(0.0, 0.5, 0.0);

    VecDataArray<int, 4> indices(1);
    indices[0] = Vec4i(0, 1, 2, 3);

    tetMesh->initialize(std::make_shared<VecDataArray<double, 3>>(vertices),
        std::make_shared<VecDataArray<int, 4>>(indices));

    createScene();
    runAllMaterials();
}