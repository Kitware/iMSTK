/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkCellMesh.h"

namespace imstk
{
///
/// \class LineMesh
///
/// \brief Base class for all volume mesh types
///
class LineMesh : public CellMesh<2>
{
public:
    LineMesh() = default;
    ~LineMesh() override = default;

    int getNumLines() const { return getNumCells(); }
    void setLinesIndices(std::shared_ptr<VecDataArray<int, 2>> indices) { setCells(indices); }
    std::shared_ptr<VecDataArray<int, 2>> getLinesIndices() const { return getCells(); }

    IMSTK_TYPE_NAME(LineMesh)

    ///
    /// \brief Polymorphic clone, hides the declaration in superclass
    /// return own type
    ///
    std::unique_ptr<LineMesh> clone()
    {
        return std::unique_ptr<LineMesh>(cloneImplementation());
    }

private:
    LineMesh* cloneImplementation() const
    {
        // Do shallow copy
        LineMesh* geom = new LineMesh(*this);
        // Deal with deep copy members
        geom->m_indices = std::make_shared<VecDataArray<int, 2>>(*m_indices);
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
