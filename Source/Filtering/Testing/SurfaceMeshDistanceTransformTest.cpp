/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "gtest/gtest.h"

#include "imstkGeometryUtilities.h"
#include "imstkImageData.h"
#include "imstkOrientedBox.h"
#include "imstkSurfaceMesh.h"
#include "imstkSurfaceMeshDistanceTransform.h"

using namespace imstk;

TEST(SurfaceMeshDistanceTransformTest, FilterWithBounds)
{
    auto mesh = GeometryUtils::toSurfaceMesh(std::make_shared<OrientedBox>());

    Vec3d lowerLeft;
    Vec3d upperRight;
    // Convert the image into SDF
    mesh->computeBoundingBox(lowerLeft, upperRight, 5.0);
    Vec6d bounds;
    bounds << lowerLeft.x(), upperRight.x(), lowerLeft.y(), upperRight.y(), lowerLeft.z(), upperRight.z();
    auto toSdf = std::make_shared<SurfaceMeshDistanceTransform>();
    toSdf->setInputMesh(mesh);

    toSdf->setBounds(bounds);

    auto dimensions = Vec3i{ 4, 5, 6 };
    toSdf->setDimensions(dimensions);
    toSdf->setTolerance(1.0e-9);
    toSdf->update();

    auto image = toSdf->getOutputImage();

    EXPECT_EQ(dimensions, image->getDimensions());
    EXPECT_TRUE(bounds.isApprox(image->getBounds()));
}

TEST(SurfaceMeshDistanceTransformTest, FilterWithoutBounds)
{
    auto mesh  = GeometryUtils::toSurfaceMesh(std::make_shared<OrientedBox>());
    auto toSdf = std::make_shared<SurfaceMeshDistanceTransform>();
    toSdf->setInputMesh(mesh);

    auto dimensions = Vec3i{ 4, 5, 6 };
    toSdf->setDimensions(dimensions);
    toSdf->setTolerance(1.0e-9);
    toSdf->update();

    auto image = toSdf->getOutputImage();

    Vec3d lowerLeft;
    Vec3d upperRight;
    mesh->computeBoundingBox(lowerLeft, upperRight);
    Vec6d bounds;
    bounds << lowerLeft.x(), upperRight.x(), lowerLeft.y(), upperRight.y(), lowerLeft.z(), upperRight.z();

    EXPECT_EQ(dimensions, image->getDimensions());
    EXPECT_TRUE(bounds.isApprox(image->getBounds()));
}