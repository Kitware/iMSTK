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
    mesh->computeBoundingBox(lowerLeft, upperRight);
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
    mesh->computeBoundingBox(lowerLeft, upperRight, toSdf->getBoundsMargin());
    Vec6d bounds;
    bounds << lowerLeft.x(), upperRight.x(), lowerLeft.y(), upperRight.y(), lowerLeft.z(), upperRight.z();

    EXPECT_EQ(dimensions, image->getDimensions());
    EXPECT_TRUE(bounds.isApprox(image->getBounds()));
}