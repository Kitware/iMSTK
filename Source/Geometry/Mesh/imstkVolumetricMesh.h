/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkCellMesh.h"

namespace imstk
{
class SurfaceMesh;

///
/// \class VolumetricMesh
///
/// \brief Base class for all volume mesh types
///
template<int N>
class VolumetricMesh : public CellMesh<N>
{
public:
    ~VolumetricMesh() override = default;

    ///
    /// \brief Compute the conforming surface mesh
    ///
    virtual std::shared_ptr<SurfaceMesh> extractSurfaceMesh() { return nullptr; }
};
} // namespace imstk