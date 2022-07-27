/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkGeometryUtilities.h"
#include "imstkImageData.h"
#include "imstkOrientedBox.h"
#include "imstkSurfaceMesh.h"
#include "imstkFastMarch.h"

#include <gtest/gtest.h>

using namespace imstk;

TEST(FastMarchTest, Filter)
{
    // Create a blank image
    auto image = std::make_shared<ImageData>();
    image->allocate(IMSTK_DOUBLE, 1, Vec3i(50, 50, 50));
    auto               scalarsPtr = std::dynamic_pointer_cast<DataArray<double>>(image->getScalars());
    DataArray<double>& scalars    = *scalarsPtr;
    scalars.fill(0.0);

    // Seed voxel in the center
    FastMarch fastMarch;
    fastMarch.setDistThreshold(5.0);
    fastMarch.setImage(image);
    std::vector<Vec3i> seeds = { Vec3i(25, 25, 25) };
    fastMarch.setSeeds(seeds);
    // Solve distances
    fastMarch.solve();

    EXPECT_EQ(scalars[image->getScalarIndex(25, 25, 25)], 0.0);

    // Check the immediate neighbors
    EXPECT_EQ(scalars[image->getScalarIndex(24, 25, 25)], 1.0);
    EXPECT_EQ(scalars[image->getScalarIndex(26, 25, 25)], 1.0);
    EXPECT_EQ(scalars[image->getScalarIndex(25, 24, 25)], 1.0);
    EXPECT_EQ(scalars[image->getScalarIndex(25, 26, 25)], 1.0);
    EXPECT_EQ(scalars[image->getScalarIndex(25, 25, 24)], 1.0);
    EXPECT_EQ(scalars[image->getScalarIndex(25, 25, 26)], 1.0);
}