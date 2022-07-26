/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMath.h"
#include "imstkLogger.h"

#include <array>

namespace imstk
{
///
/// \brief Class for handling data in 3D grid
/// \param Real The floating point type for computation (must be float or double
/// \param CellData The data type that each cell will hold an instance of it
///
template<class CellData>
class UniformSpatialGrid
{
public:
    ///
    /// \brief Construct a default grid ([0, 1]^3) with cell size of 1
    ///
    UniformSpatialGrid() : UniformSpatialGrid(Vec3d::Zero(), Vec3d(1.0, 1.0, 1.0), double(1.0))
    {}

    ///
    /// \brief Construct a grid with given corners and cell size
    /// \param lowerCorner The lower corner of the grid
    /// \param upperCorner The upper corner of the grid
    /// \param cellSize The length of grid cell
    ///
    UniformSpatialGrid(const Vec3d& lowerCorner, const Vec3d& upperCorner, double cellSize)
    {
        initialize(lowerCorner, upperCorner, cellSize);
    }

    ///
    /// \brief Initialize the grid data
    /// \param lowerCorner The lower corner of the grid
    /// \param upperCorner The upper corner of the grid
    /// \param cellSize the edge length of grid cell
    ///
    void initialize(const Vec3d& lowerCorner, const Vec3d& upperCorner, const double cellSize)
    {
        CHECK(cellSize > 0) << "Invalid cell size";

        m_LowerCorner = lowerCorner;
        m_UpperCorner = upperCorner;

        m_CellSize    = cellSize;
        m_InvCellSize = 1.0 / m_CellSize;

        m_NTotalCells = 1u;
        for (int i = 0; i < 3; ++i)
        {
            m_Resolution[i] = static_cast<unsigned int>(std::ceil((m_UpperCorner[i] - m_LowerCorner[i]) / m_CellSize));
            m_NTotalCells  *= m_Resolution[i];
        }

        CHECK(m_NTotalCells != 0) << "Invalid grid size: [" +
            std::to_string(m_LowerCorner[0]) + ", " + std::to_string(m_LowerCorner[1]) + ", " + std::to_string(m_LowerCorner[2]) + "] => " +
            std::to_string(m_UpperCorner[0]) + ", " + std::to_string(m_UpperCorner[1]) + ", " + std::to_string(m_UpperCorner[2]) + "], " +
            "cellSize = " + std::to_string(m_CellSize);

        // cell data must be resized to match with the number of cells
        m_CellData.resize(m_NTotalCells);
    }

    ///
    /// \brief Get number of grid cell in 3 dimensions: (num_cell_x, num_cell_y, num_cell_z)
    ///
    std::array<unsigned int, 3> getResolution() const { return m_Resolution; }

    ///
    /// \brief Get number of total cells in the grid
    ///
    unsigned int getNumTotalCells() const { return m_NTotalCells; }

    ///
    /// \brief Check if cell index in dimension d is valid (d = 0/1/2 => x/y/z dimension)
    ///
    template<int d>
    bool isValidCellIndex(const int idx) const
    { return idx >= 0 && static_cast<unsigned int>(idx) < m_Resolution[d]; }

    ///
    /// \brief Check if 3D cell indices are valid
    ///
    bool isValidCellIndices(const int i, const int j, const int k) const
    { return isValidCellIndex<0>(i) && isValidCellIndex<1>(j) && isValidCellIndex<2>(k); }

    ///
    /// \brief Get the 3D index (cell_x, cell_y, cell_z) of the cell containing the given positions
    ///
    template<class IndexType>
    std::array<IndexType, 3> getCell3DIndices(const Vec3d& ppos) const
    {
        std::array<IndexType, 3> cellIdx;
        for (int d = 0; d < 3; ++d)
        {
            cellIdx[d] = static_cast<IndexType>((ppos[d] - m_LowerCorner[d]) * m_InvCellSize);
        }
        return cellIdx;
    }

    ///
    /// \brief Get all cell data
    ///
    std::vector<CellData>& getAllCellData() { return m_CellData; }

    ///
    /// \brief Get all cell data
    ///
    const std::vector<CellData>& getAllCellData() const { return m_CellData; }

    ///
    /// \brief Get data in a cell
    /// \param A position in space
    ///
    CellData& getCellData(const Vec3d& ppos)
    {
        return m_CellData[getCellLinearizedIndex<unsigned int>(ppos)];
    }

    ///
    /// \brief Get data in a cell
    /// \param A position in space
    ///
    const CellData& getCellData(const Vec3d& ppos) const
    { return m_CellData[getCellLinearizedIndex<unsigned int>(ppos)]; }

    ///
    /// \brief Get data in a cell
    /// \param A linearized index of cell
    ///
    CellData& getCellData(size_t linearizedIdx)
    {
        assert(linearizedIdx < m_CellData.size());
        return m_CellData[linearizedIdx];
    }

    ///
    /// \brief Get data in a cell
    /// \param A linearized index of cell
    ///
    const CellData& getCellData(size_t linearizedIdx) const
    {
        assert(linearizedIdx < m_CellData.size());
        return m_CellData[linearizedIdx];
    }

    ///
    /// \brief Get data in a cell
    /// \param 3D index of a cell
    ///
    template<class IndexType>
    CellData& getCellData(const std::array<IndexType, 3>& cellIdx)
    {
        return m_CellData[getCellLinearizedIndex(cellIdx[0], cellIdx[1], cellIdx[2])];
    }

    ///
    /// \brief Get data in a cell
    /// \param 3D index of a cell
    ///
    template<class IndexType>
    const CellData& getCellData(const std::array<IndexType, 3>& cellIdx) const
    {
        return m_CellData[getCellLinearizedIndex(cellIdx[0], cellIdx[1], cellIdx[2])];
    }

    ///
    /// \brief Get data in a cell
    /// \param 3D index of a cell
    ///
    template<class IndexType>
    CellData& getCellData(const IndexType i, const IndexType j, const IndexType k)
    {
        return m_CellData[getCellLinearizedIndex(i, j, k)];
    }

    ///
    /// \brief Get data in a cell
    /// \param 3D index of a cell
    ///
    template<class IndexType>
    const CellData& getCellData(const IndexType i, const IndexType j, const IndexType k) const
    {
        return m_CellData[getCellLinearizedIndex(i, j, k)];
    }

    ///
    /// \brief Apply a function to all cell data
    ///
    template<class Function>
    void loopAllCellData(Function&& func)
    {
        for (auto& cellData: m_CellData)
        {
            func(cellData);
        }
    }

    ///
    /// \brief Get linearized index from cell 3D indices: index in 3D (cell_x, cell_y, cell_z) => index in 1D
    /// \param 3D index of a cell
    ///
    template<class IndexType>
    IndexType getCellLinearizedIndex(const IndexType i, const IndexType j, const IndexType k) const
    {
        auto flatIndex = (k * static_cast<IndexType>(m_Resolution[1]) + j) * static_cast<IndexType>(m_Resolution[0]) + i;
        assert(flatIndex < static_cast<IndexType>(m_NTotalCells));
        return flatIndex;
    }

    ///
    /// \brief Get linearized index of cell containing the given position
    /// \param A position in space
    ///
    template<class IndexType>
    IndexType getCellLinearizedIndex(const Vec3d& ppos) const
    {
        auto cellIdx = getCell3DIndices<IndexType>(ppos);
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
        LOG_IF(FATAL, (!isValidCellIndices(cellIdx[0], cellIdx[1], cellIdx[2]))) <<
            "Invalid cell indices: " +
            std::to_string(cellIdx[0]) + "/" + std::to_string(m_Resolution[0]) + ", " +
            std::to_string(cellIdx[1]) + "/" + std::to_string(m_Resolution[1]) + ", " +
            std::to_string(cellIdx[2]) + "/" + std::to_string(m_Resolution[2]);
#endif
        return getCellLinearizedIndex<IndexType>(cellIdx[0], cellIdx[1], cellIdx[2]);
    }

private:
    Vec3d  m_LowerCorner;                     ///< Lower corner of the grid
    Vec3d  m_UpperCorner;                     ///< Upper corner of the grid
    double m_CellSize;                        ///< Length of grid cell
    double m_InvCellSize;                     ///< Inverse length of grid cell

    unsigned int m_NTotalCells;               ///< Number of total cells
    std::array<unsigned int, 3> m_Resolution; ///< Grid resolution (number of cells in x/y/z dimensions)

    std::vector<CellData> m_CellData;         ///< Data stored for each cell
};
} // namespace imstk