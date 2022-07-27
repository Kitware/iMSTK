/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMath.h"
#include "imstkVecDataArray.h"

namespace imstk
{
class GridBasedNeighborSearch;
class SpatialHashTableSeparateChaining;

///
/// \class NeighborSearch
/// \brief A wrapper class for Grid-based and spatial-hashing neighbor search
///
class NeighborSearch
{
public:
    enum class Method
    {
        UniformGridBasedSearch,
        SpatialHashing
    };

    ///
    /// \brief Constructor
    /// \param The selected search method
    ///
    NeighborSearch(Method searchMethod, double searchRadius = 0.0);

    ///
    /// \brief Set the search radius
    ///
    void setSearchRadius(const double searchRadius);

    ///
    /// \brief Get the current search radius
    ///
    double getSearchRadius() const { return m_SearchRadius; }

    ///
    /// \brief Search neighbors for each points within the search radius
    /// \param points The given points to search for neighbors
    /// \return List of list of neighbor indices for each point
    ///
    std::vector<std::vector<size_t>> getNeighbors(const VecDataArray<double, 3>& points);

    ///
    /// \brief Search neighbors for each point within the search radius
    /// \param result The list of lists of neighbor indices for each point
    /// \param points The given points to search for neighbors
    ///
    void getNeighbors(std::vector<std::vector<size_t>>& result, const VecDataArray<double, 3>& points);

    ///
    /// \brief Search neighbors from setB for each point in setA within the search radius. SetA and setB can be different.
    /// \param result The list of lists of neighbors for each point
    /// \param setA The point set for which performing neighbor search
    /// \param setB The point set where neighbor indices will be collected
    ///
    void getNeighbors(std::vector<std::vector<size_t>>& result, const VecDataArray<double, 3>& setA, const VecDataArray<double, 3>& setB);

private:
    Method m_Method;
    double m_SearchRadius = 0.0;

    std::shared_ptr<GridBasedNeighborSearch> m_GridBasedSearcher;
    std::shared_ptr<SpatialHashTableSeparateChaining> m_SpatialHashSearcher;
};
} // namespace imstk