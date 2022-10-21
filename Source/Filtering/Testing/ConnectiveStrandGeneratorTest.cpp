/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkGeometryUtilities.h"
#include "imstkOrientedBox.h"
#include "imstkSurfaceMesh.h"
#include "imstkLineMesh.h"
#include "imstkConnectiveStrandGenerator.h"

#include <gtest/gtest.h>

using namespace imstk;

TEST(ConnectiveStrandGenerator, Filter)
{
    // Create two surface meshes
    std::shared_ptr<SurfaceMesh> meshA =
        GeometryUtils::toTriangleGrid(Vec3d::Zero(),
            Vec2d(0.1, 0.1), Vec2i(2, 2));
    meshA->rotate(Vec3d(0.0, 0.0, 1.0), (90.0) / 180.0 * 3.14, Geometry::TransformType::ApplyToData);
    meshA->translate(Vec3d(0.25, 0, 0), Geometry::TransformType::ApplyToData);

    std::shared_ptr<SurfaceMesh> meshB =
        GeometryUtils::toTriangleGrid(Vec3d::Zero(),
            Vec2d(0.1, 0.1), Vec2i(2, 2));
    meshB->rotate(Vec3d(0.0, 0.0, 1.0), (-90.0) / 180.0 * 3.14, Geometry::TransformType::ApplyToData);
    meshB->translate(Vec3d(-0.25, 0, 0), Geometry::TransformType::ApplyToData);

    //// Create ConnectiveStrandGenerator
    auto testConnector = std::make_shared<ConnectiveStrandGenerator>();
    testConnector->setInputMeshes(meshA, meshB);
    testConnector->update();

    // Test with default options
    auto lineMesh1 = std::dynamic_pointer_cast<LineMesh>(testConnector->getOutput(0));
    EXPECT_EQ(6, lineMesh1->getNumCells());
    EXPECT_EQ(8, lineMesh1->getNumVertices());

    testConnector->setSegmentsPerStrand(1);
    testConnector->setStrandsPerFace(3);
    testConnector->update();
    auto lineMesh2 = std::dynamic_pointer_cast<LineMesh>(testConnector->getOutput(0));
    EXPECT_EQ(6, lineMesh2->getNumCells());
    EXPECT_EQ(12, lineMesh2->getNumVertices());
}