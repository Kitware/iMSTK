/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkGeometryAlgorithm.h"

namespace imstk
{
class LineMesh;
class SurfaceMesh;

///
/// \class ExtractEdges
///
/// \brief This filter extracts the edges of a SurfaceMesh producing a LineMesh
///
class ExtractEdges : public GeometryAlgorithm
{
public:
    ExtractEdges();
    ~ExtractEdges() override = default;

    std::shared_ptr<LineMesh> getOutputMesh() const;

    void setInputMesh(std::shared_ptr<SurfaceMesh> inputMesh);

protected:
    void requestUpdate() override;
};
} // namespace imstk