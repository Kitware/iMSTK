/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkGeometryAlgorithm.h"

namespace imstk
{
class ImageData;

///
/// \class ImageGradient
///
/// \brief This filter computes the gradient or magnitude using sobels kernel over an image
///
class ImageGradient : public GeometryAlgorithm
{
public:
    ImageGradient();
    ~ImageGradient() override = default;

public:
    ///
    /// \brief Required input, port 0
    ///
    void setInputImage(std::shared_ptr<ImageData> inputImage);

    ///
    /// \brief Get/Set whether to compute the magnitude
    /// If on, a single channel magnitude image is output, if off, a 3 component gradient is produced
    ///@{
    imstkSetMacro(ComputeMagnitude, bool);
    imstkGetMacro(ComputeMagnitude, bool);
///@}

protected:
    void requestUpdate() override;

private:
    bool m_ComputeMagnitude;
};
} // namespace imstk