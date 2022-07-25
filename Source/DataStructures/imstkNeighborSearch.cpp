/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkGridBasedNeighborSearch.h"
#include "imstkSpatialHashTableSeparateChaining.h"
#include "imstkNeighborSearch.h"
#include "imstkParallelUtils.h"

namespace imstk
{
NeighborSearch::NeighborSearch(NeighborSearch::Method searchMethod, double searchRadius /*= 0*/) :
    m_Method(searchMethod), m_SearchRadius(searchRadius)
{
    if (m_Method == Method::UniformGridBasedSearch)
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

void
NeighborSearch::setSearchRadius(const double searchRadius)
{
    m_SearchRadius = searchRadius;
    if (m_Method == Method::UniformGridBasedSearch)
    {
        m_GridBasedSearcher->setSearchRadius(m_SearchRadius);
    }
    else
    {
        m_SpatialHashSearcher->setCellSize(m_SearchRadius, m_SearchRadius, m_SearchRadius);
    }
}

std::vector<std::vector<size_t>>
NeighborSearch::getNeighbors(const VecDataArray<double, 3>& points)
{
    std::vector<std::vector<size_t>> result;
    getNeighbors(result, points, points);
    return result;
}

void
NeighborSearch::getNeighbors(std::vector<std::vector<size_t>>& result, const VecDataArray<double, 3>& points)
{
    getNeighbors(result, points, points);
}

void
NeighborSearch::getNeighbors(std::vector<std::vector<size_t>>& result, const VecDataArray<double, 3>& setA, const VecDataArray<double, 3>& setB)
{
    if (m_Method == Method::UniformGridBasedSearch)
    {
        m_GridBasedSearcher->getNeighbors(result, setA, setB);
    }
    else
    {
        m_SpatialHashSearcher->clear();
        m_SpatialHashSearcher->insertPoints(setB);

        ParallelUtils::parallelFor(setA.size(),
            [&](const size_t p) {
                // For each point in setA, find neighbors in setB
                m_SpatialHashSearcher->getPointsInSphere(result[p], setA[p], m_SearchRadius);
            });
    }
}
} // namespace imstk