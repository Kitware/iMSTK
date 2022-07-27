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
/// \class ImageDistanceTransform
///
/// \brief This filter generates a signed or unsigned distance transform from a binary mask
///
class ImageDistanceTransform : public GeometryAlgorithm
{
public:
    ImageDistanceTransform();
    virtual ~ImageDistanceTransform() override = default;

public:
    std::shared_ptr<ImageData> getOutputImage() const;

    ///
    /// \brief Required input, port 0
    ///
    void setInputImage(std::shared_ptr<ImageData> refImage);

    imstkGetMacro(UseUnsigned, bool);
    imstkSetMacro(UseUnsigned, bool);

protected:
    void requestUpdate() override;

private:
    bool m_UseUnsigned = false;
};
} // namespace imstk