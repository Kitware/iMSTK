/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkImageData.h"
#include "imstkVecDataArray.h"

#include <gtest/gtest.h>
#include <memory>

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
} // namespace

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
