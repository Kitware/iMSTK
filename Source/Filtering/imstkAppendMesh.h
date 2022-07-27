/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkGeometryAlgorithm.h"

namespace imstk
{
class SurfaceMesh;

///
/// \class AppendMesh
///
/// \brief This filter appends two SurfaceMeshes, no topological connections are made
///
class AppendMesh : public GeometryAlgorithm
{
public:
    AppendMesh();
    ~AppendMesh() override = default;

    void addInputMesh(std::shared_ptr<SurfaceMesh> inputMesh);
    std::shared_ptr<SurfaceMesh> getOutputMesh() const;

protected:
    void requestUpdate() override;
};
} // namespace imstk