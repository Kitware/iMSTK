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
class SurfaceMesh;

///
/// \class SurfaceMeshFlyingEdges
///
/// \brief This filter extracts a single isocontour from an imstkImageData
///
class SurfaceMeshFlyingEdges : public GeometryAlgorithm
{
public:
    SurfaceMeshFlyingEdges();
    ~SurfaceMeshFlyingEdges() override = default;

public:
    ///
    /// \brief Required input, port 0
    ///
    void setInputImage(std::shared_ptr<ImageData> inputImage);

    std::shared_ptr<SurfaceMesh> getOutputMesh() const;

    ///
    /// \brief Get the value at which the surface should be produced
    ///@{
    imstkSetMacro(IsoValue, double);
    imstkGetMacro(IsoValue, double);
///@}

protected:
    void requestUpdate() override;

private:
    double m_IsoValue = 0.0;
};
} // namespace imstk