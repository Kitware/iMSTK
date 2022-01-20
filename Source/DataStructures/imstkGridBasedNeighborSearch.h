/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#pragma once

#include "imstkSpinLock.h"
#include "imstkUniformSpatialGrid.h"
#include "imstkVecDataArray.h"

namespace imstk
{
///
/// \brief Class for searching neighbors using regular grid
///
class GridBasedNeighborSearch
{
public:
    GridBasedNeighborSearch() = default;

    ///
    /// \brief Construct class with search radius
    /// \param radius The search radius
    ///
    GridBasedNeighborSearch(const double radius) : m_SearchRadius(radius), m_SearchRadiusSqr(radius * radius) {}

    ///
    /// \brief Set the search radius
    /// \param radius The search radius
    ///
    void setSearchRadius(const double radius);

    ///
    /// \brief Get the search radius
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
    double m_SearchRadius    = 0.0;
    double m_SearchRadiusSqr = 0.0;

    // Data store in each grid cell
    // This entire struct can be replaced by tbb::concurrent_vector<size_t>, however, with lower performance
    struct CellData
    {
        std::vector<size_t> particleIndices; // Store list of particles
        ParallelUtils::SpinLock lock;        // An atomic lock for thread-safe writing
    };
    UniformSpatialGrid<CellData> m_Grid;
};
} // namespace imstk