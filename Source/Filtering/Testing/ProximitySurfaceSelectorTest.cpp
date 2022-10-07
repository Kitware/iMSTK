/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkGeometryUtilities.h"
#include "imstkOrientedBox.h"
#include "imstkSurfaceMesh.h"
#include "imstkProximitySurfaceSelector.h"

#include <gtest/gtest.h>

using namespace imstk;

TEST(ProximitySurfaceSelector, Filter)
{
    // Create two surface meshes
    const Vec3d&                 size1     = Vec3d(4.0, 4.0, 4.0);
    const Vec3d&                 center1   = Vec3d(-2.5, 0.0, 0.0);
    auto                         cubeGeom1 = std::make_shared<OrientedBox>(size1, center1);
    std::shared_ptr<SurfaceMesh> surfMesh1 = GeometryUtils::toSurfaceMesh(cubeGeom1);

    const Vec3d&                 size2     = Vec3d(4.0, 4.0, 4.0);
    const Vec3d&                 center2   = Vec3d(2.5, 0.0, 0.0);
    auto                         cubeGeom2 = std::make_shared<OrientedBox>(size2, center2);
    std::shared_ptr<SurfaceMesh> surfMesh2 = GeometryUtils::toSurfaceMesh(cubeGeom2);

    // Create ProximitySurfaceSelector
    auto proxSelector = std::make_shared<ProximitySurfaceSelector>();

    proxSelector->setInputMeshes(surfMesh1, surfMesh2);
    double maxDist = 1.1;
    proxSelector->setProximity(maxDist);
    proxSelector->update();

    auto subMeshA = std::dynamic_pointer_cast<SurfaceMesh>(proxSelector->getOutput(0));
    auto subMeshB = std::dynamic_pointer_cast<SurfaceMesh>(proxSelector->getOutput(1));

    EXPECT_EQ(12, subMeshA->getNumCells());
    EXPECT_EQ(subMeshA->getNumCells(), subMeshB->getNumCells());
    EXPECT_EQ(subMeshA->getNumVertices(), subMeshB->getNumVertices());
}