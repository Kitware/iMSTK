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
/// \class SurfaceMeshTextureProject
///
/// \brief This filter projects vertex texture attributes from one SurfaceMesh to
/// another by finding the closest point on the source mesh and interpolating the
/// results. This could later be expanded to arbitrary vertex attributes
///
class SurfaceMeshTextureProject : public GeometryAlgorithm
{
public:
    SurfaceMeshTextureProject();
    ~SurfaceMeshTextureProject() override = default;

public:
    ///
    /// \brief The mesh with attribute to put on the other
    ///
    void setSourceMesh(std::shared_ptr<SurfaceMesh> surfMesh);

    ///
    /// \brief The mesh to recieve the attribute
    ///
    void setDestMesh(std::shared_ptr<SurfaceMesh> destMesh);

    ///
    /// \brief destMesh copy with the attribute
    ///
    std::shared_ptr<SurfaceMesh> getOutputMesh();

protected:
    void requestUpdate() override;
};
} // namespace imstk