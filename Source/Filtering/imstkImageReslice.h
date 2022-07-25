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
/// \class ImageReslice
///
/// \brief Resamples an image using a transform
///
class ImageReslice : public GeometryAlgorithm
{
public:
    enum class InterpolateType
    {
        Linear,
        Cubic,
        NearestNeighbor
    };

public:
    ImageReslice();
    ~ImageReslice() override = default;

public:
    std::shared_ptr<ImageData> getOutputImage() const;

    void setInputImage(std::shared_ptr<ImageData> inputData);

    ///
    /// \brief Get/Set the transformation matrix
    ///@{
    imstkSetMacro(Transform, const Mat4d&);
    imstkGetMacro(Transform, const Mat4d&);
    ///@}

    ///
    /// \brief Set the interpolation type to use when resampling
    ///@{
    imstkSetMacro(InterpolationType, InterpolateType);
    imstkGetMacro(InterpolationType, InterpolateType);
///@}

protected:
    void requestUpdate() override;

private:
    Mat4d m_Transform;
    InterpolateType m_InterpolationType = InterpolateType::Linear;
};
} // namespace imstk