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

#include "imstkGridBasedNeighborSearch.h"
#include "imstkSpatialHashTableSeparateChaining.h"
#include "imstkNeighborSearch.h"
#include "imstkParallelHelpers.h"

namespace imstk
{
NeighborSearch::NeighborSearch(NeighborSearch::Method searchMethod, Real searchRadius /*= 0*/) :
    m_Method(searchMethod), m_SearchRadius(searchRadius)
{
    if(m_Method == Method::UniformGridBasedSearch)
    {
        m_GridBasedSearcher = std::make_shared<GridBasedNeighborSearch>();
        m_GridBasedSearcher->setSearchRadius(m_SearchRadius);
    }
    else
    {
        m_SpatialHashSearcher = std::make_shared<SpatialHashTableSeparateChaining>();
        m_SpatialHashSearcher->setCellSize(m_SearchRadius, m_SearchRadius, m_SearchRadius);
    }
}

void NeighborSearch::setSearchRadius(const Real searchRadius)
{
    m_SearchRadius = searchRadius;
    if(m_Method == Method::UniformGridBasedSearch)
    {
        m_GridBasedSearcher->setSearchRadius(m_SearchRadius);
    }
    else
    {
        m_SpatialHashSearcher->setCellSize(m_SearchRadius, m_SearchRadius, m_SearchRadius);
    }
}

std::vector<std::vector<size_t>>
NeighborSearch::getNeighbors(const StdVectorOfVec3r& points)
{
    std::vector<std::vector<size_t>> result;
    getNeighbors(result, points, points);
    return result;
}

void NeighborSearch::getNeighbors(std::vector<std::vector<size_t>>&result, const StdVectorOfVec3r& points)
{
    getNeighbors(result, points, points);
}

void NeighborSearch::getNeighbors(std::vector<std::vector<size_t>>& result, const StdVectorOfVec3r& setA, const StdVectorOfVec3r& setB)
{
    if(m_Method == Method::UniformGridBasedSearch)
    {
        m_GridBasedSearcher->getNeighbors(result, setA, setB);
    }
    else
    {
        m_SpatialHashSearcher->clear();
        m_SpatialHashSearcher->insertPoints(setB);

        imstk_parallel_for(setA.size(),
            [&](const size_t p) {
                // For each point in setA, find neighbors in setB
                m_SpatialHashSearcher->getPointsInSphere(result[p], setA[p], m_SearchRadius);
            });
    }
}
} // end namespace imstk
