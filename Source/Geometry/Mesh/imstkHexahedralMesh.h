/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMacros.h"
#include "imstkVolumetricMesh.h"

namespace imstk
{
///
/// \class HexahedralMesh
///
/// \brief Hexahedral mesh class
///
class HexahedralMesh : public VolumetricMesh<8>
{
public:
    HexahedralMesh() = default;
    ~HexahedralMesh() override = default;

    IMSTK_TYPE_NAME(HexahedralMesh)

    ///
    /// \brief Clear all the mesh data
    ///
    void clear() override;

    ///
    /// \brief Print the hexahedral mesh
    ///
    void print() const override;

    ///
    /// \brief Extract surface Mesh
    std::shared_ptr<SurfaceMesh> extractSurfaceMesh() override;

// Accessors
    ///
    /// \brief Returns the number of hexahedra
    ///
    int getNumHexahedra() const { return getNumCells(); }

    ///
    /// \brief Compute and return the volume of the hexahedral mesh
    ///
    double getVolume() override;
};
} // namespace imstk