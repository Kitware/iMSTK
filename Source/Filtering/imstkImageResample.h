/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkGeometryAlgorithm.h"
#include "imstkMath.h"

namespace imstk
{
class ImageData;

///
/// \class ImageResample
///
/// \brief Trilinearly Resamples a 3d image to different dimensions
///
class ImageResample : public GeometryAlgorithm
{
public:
    ImageResample();
    ~ImageResample() override = default;

public:
    std::shared_ptr<ImageData> getOutputImage() const;

    void setInputImage(std::shared_ptr<ImageData> inputData);

    imstkGetMacro(Dimensions, const Vec3i&);

    imstkSetMacro(Dimensions, const Vec3i&);

protected:
    void requestUpdate() override;

private:
    Vec3i m_Dimensions;
};
} // namespace imstk