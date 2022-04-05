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