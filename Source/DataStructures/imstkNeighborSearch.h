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

#include <memory>
#include "imstkMath.h"

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
    explicit NeighborSearch(Method searchMethod, Real searchRadius = 0);

    ///
    /// \brief Set the search radius
    ///
    void setSearchRadius(const Real searchRadius);

    ///
    /// \brief Get the current search radius
    ///
    Real getSearchRadius() const { return m_SearchRadius; }

    ///
    /// \brief Search neighbors for each points within the search radius
    /// \param points The given points to search for neighbors
    /// \return List of list of neighbor indices for each point
    ///
    std::vector<std::vector<size_t>> getNeighbors(const StdVectorOfVec3r& points);

    ///
    /// \brief Search neighbors for each point within the search radius
    /// \param result The list of lists of neighbor indices for each point
    /// \param points The given points to search for neighbors
    ///
    void getNeighbors(std::vector<std::vector<size_t>>& result, const StdVectorOfVec3r& points);

    ///
    /// \brief Search neighbors from setB for each point in setA within the search radius. SetA and setB can be different.
    /// \param result The list of lists of neighbors for each point
    /// \param setA The point set for which performing neighbor search
    /// \param setB The point set where neighbor indices will be collected
    ///
    void getNeighbors(std::vector<std::vector<size_t>>& result, const StdVectorOfVec3r& setA, const StdVectorOfVec3r& setB);

private:
    Method m_Method;
    Real   m_SearchRadius = 0;

    std::shared_ptr<GridBasedNeighborSearch> m_GridBasedSearcher;
    std::shared_ptr<SpatialHashTableSeparateChaining> m_SpatialHashSearcher;
};
} // end namespace imstk
