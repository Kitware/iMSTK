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

    ///
    /// \brief Polymorphic clone, hides the declaration in superclass
    /// return own type
    ///
    std::unique_ptr<HexahedralMesh> clone()
    {
        return std::unique_ptr<HexahedralMesh>(cloneImplementation());
    }

private:
    HexahedralMesh* cloneImplementation() const
    {
        // Do shallow copy
        HexahedralMesh* geom = new HexahedralMesh(*this);
        // Deal with deep copy members
        geom->m_indices = std::make_shared<VecDataArray<int, 8>>(*m_indices);
        for (auto i : m_cellAttributes)
        {
            geom->m_cellAttributes[i.first] = i.second->clone();
        }
        geom->m_initialVertexPositions = std::make_shared<VecDataArray<double, 3>>(*m_initialVertexPositions);
        geom->m_vertexPositions = std::make_shared<VecDataArray<double, 3>>(*m_vertexPositions);
        for (auto i : m_vertexAttributes)
        {
            geom->m_vertexAttributes[i.first] = i.second->clone();
        }
        return geom;
    }
};
} // namespace imstk