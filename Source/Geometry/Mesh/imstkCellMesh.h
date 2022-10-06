/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkAbstractCellMesh.h"

#pragma once

namespace imstk
{
///
/// \class CellMesh
///
/// \brief Abstract template base class for all meshes that have homogenous
/// cell types. This class allows templated access to cells. A cell in iMSTK
/// could be a line, triangle, quad, tetrahedron, hexahedron, ... It is a
/// group of vertices that form an element of a larger mesh.
/// \tparam N number of of vertices in cell
///
template<int N>
class CellMesh : public AbstractCellMesh
{
public:
    static constexpr int CellVertexCount = N;
    using VertexDataType = VecDataArray<double, 3>;
    using CellIndexType  = VecDataArray<int, N>;
    using CellType       = Eigen::Matrix<int, N, 1>;

    CellMesh() : m_indices(std::make_shared<VecDataArray<int, N>>()) { }
    ~CellMesh() override = default;

    ///
    /// \brief Initializes the rest of the data structures given vertex
    /// positions and connectivity
    ///
    void initialize(std::shared_ptr<VecDataArray<double, 3>> vertices,
                    std::shared_ptr<VecDataArray<int, N>> indices)
    {
        clear();

        PointSet::initialize(vertices);
        setCells(indices);
    }

    void clear() override
    {
        AbstractCellMesh::clear();
        if (m_indices != nullptr)
        {
            m_indices->clear();
        }
    }

    ///
    /// \brief Computes neighboring triangles for all vertices
    ///
    void computeVertexToCellMap() override
    {
        m_vertexToCells.clear();
        m_vertexToCells.resize(m_vertexPositions->size());

        int cellId = 0;
        for (const auto& cell : *m_indices)
        {
            // \todo: Could variadic unfold
            // Subclasses can implement a more efficient version if needbe
            for (int i = 0; i < N; i++)
            {
                m_vertexToCells.at(cell[i]).insert(cellId);
            }
            cellId++;
        }
    }

    ///
    /// \brief Computes neighboring vertices for all vertices
    ///
    void computeVertexNeighbors() override
    {
        m_vertexToNeighborVertex.clear();
        m_vertexToNeighborVertex.resize(m_vertexPositions->size());
        this->computeVertexToCellMap();

        // For every vertex
        const VecDataArray<int, N>& indices = *m_indices;
        for (size_t vertexId = 0; vertexId < m_vertexToNeighborVertex.size(); vertexId++)
        {
            // For every cell it is connected too
            for (const int cellId : m_vertexToCells.at(vertexId))
            {
                // For every vertex of that cell
                for (int i = 0; i < N; i++)
                {
                    // So long as its not the source vertex (not a neighbor of itself)
                    const int vertexId2 = indices[cellId][i];
                    if (vertexId2 != static_cast<int>(vertexId))
                    {
                        m_vertexToNeighborVertex.at(vertexId).insert(vertexId2);
                    }
                }
            }
        }
    }

    ///
    /// \brief compute the barycentric weights of a given point in 3D space for a given the cell
    ///
    virtual Eigen::Vector<double, N> computeBarycentricWeights(const int    imstkNotUsed(cellId),
                                                               const Vec3d& imstkNotUsed(pos)) const
    {
        return Eigen::Vector<double, N>::Zero();
    }

    std::shared_ptr<AbstractDataArray> getAbstractCells() const override { return m_indices; }

    ///
    /// \brief Get/Set cell connectivity
    ///@{
    void setCells(std::shared_ptr<VecDataArray<int, N>> indices) { m_indices = indices; }
    std::shared_ptr<VecDataArray<int, N>> getCells() const { return m_indices; }
    ///@}

    ///
    /// \brief Returns the number of cells
    ///
    int getNumCells() const override { return m_indices->size(); }

    ///
    /// \brief Polymorphic clone, hides the declaration in superclass
    /// return own type
    ///
    std::unique_ptr<CellMesh<N>> clone()
    {
        return std::unique_ptr<CellMesh<N>>(cloneImplementation());
    }

protected:
    std::shared_ptr<VecDataArray<int, N>> m_indices = nullptr;

private:
    CellMesh<N>* cloneImplementation() const
    {
        // Do shallow copy
        CellMesh<N>* geom = new CellMesh<N>(*this);
        // Deal with deep copy members
        geom->m_indices = std::make_shared<VecDataArray<int, N>>(*m_indices);
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