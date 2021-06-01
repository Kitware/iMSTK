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

#include <memory>

#include "imstkImageData.h"
#include "imstkVecDataArray.h"
#include "imstkMath.h"

using namespace imstk;
namespace
{
VecDataArray<double, 3> doubleArray = {
    Vec3d{ 0, 0, 0 },
    Vec3d{ 1, 1, 1 },
    Vec3d{ 2, 2, 2 },
    Vec3d{ 3, 3, 3 }
};

DataArray<int>    intScalars    = { 1, 2, 3, 4 };
DataArray<double> doubleScalars = { 1.0, 2.0, 3.0, 4.0 };
}

TEST(imstkImageDataTest, Cast)
{
    Vec3i     dim     = { 2, 2, 0 };
    Vec3d     spacing = { 1.0, 2.0, 3.0 };
    Vec3d     origin  = { 4.0, 5.0, 6.0 };
    ImageData img;
    img.setScalars(std::make_shared<DataArray<int>>(intScalars), 1, dim.data());
    img.setSpacing(spacing);
    img.setOrigin(origin);
    EXPECT_EQ(IMSTK_INT, img.getScalarType());

    auto castImg    = img.cast(IMSTK_DOUBLE);
    auto newScalars = castImg->getScalars();
    EXPECT_EQ(IMSTK_DOUBLE, newScalars->getScalarType());
    auto actualScalars = std::dynamic_pointer_cast<DataArray<double>>(newScalars);
    EXPECT_TRUE(actualScalars != nullptr);
    EXPECT_TRUE(dim.isApprox(castImg->getDimensions()));
    EXPECT_TRUE(spacing.isApprox(castImg->getSpacing()));
    EXPECT_TRUE(origin.isApprox(castImg->getOrigin()));

    for (int i = 0; i < actualScalars->size(); ++i)
    {
        EXPECT_DOUBLE_EQ(static_cast<double>(intScalars[i]), (*actualScalars)[i]);
    }
}
