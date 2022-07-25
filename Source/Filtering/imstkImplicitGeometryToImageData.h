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
class ImplicitGeometry;

///
/// \class ImplicitGeometryToImageData
///
/// \brief This filter rasterizes an implicit function to image of
/// specified dimensions and bounds
///
class ImplicitGeometryToImageData : public GeometryAlgorithm
{
public:
    ImplicitGeometryToImageData();
    ~ImplicitGeometryToImageData() override = default;

public:
    std::shared_ptr<ImageData> getOutputImage() const;

    ///
    /// \brief Required input, port 0
    ///
    void setInputGeometry(std::shared_ptr<ImplicitGeometry> inputGeometry);

    imstkGetMacro(Dimensions, const Vec3i&);
    imstkGetMacro(Bounds, const Vec6d&);

    imstkSetMacro(Dimensions, const Vec3i&);
    imstkSetMacro(Bounds, const Vec6d&);

protected:
    void requestUpdate() override;

private:
    Vec3i m_Dimensions;
    Vec6d m_Bounds;
};
} // namespace imstk