/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkGridBasedNeighborSearch.h"
#include "imstkParallelUtils.h"

namespace imstk
{
void
GridBasedNeighborSearch::setSearchRadius(const double radius)
{
    m_SearchRadius    = radius;
    m_SearchRadiusSqr = radius * radius;
}

std::vector<std::vector<size_t>>
GridBasedNeighborSearch::getNeighbors(const VecDataArray<double, 3>& points)
{
    std::vector<std::vector<size_t>> result;
    getNeighbors(result, points, points);
    return result;
}

void
GridBasedNeighborSearch::getNeighbors(std::vector<std::vector<size_t>>& result, const VecDataArray<double, 3>& points)
{
    getNeighbors(result, points, points);
}

void
GridBasedNeighborSearch::getNeighbors(std::vector<std::vector<size_t>>& result, const VecDataArray<double, 3>& setA, const VecDataArray<double, 3>& setB)
{
    LOG_IF(FATAL, (std::abs(m_SearchRadius) < 1e-8)) << "Neighbor search radius is zero";

    // firstly compute the bounding box of points in setB
    Vec3d lowerCorner;
    Vec3d upperCorner;
    ParallelUtils::findAABB(setB, lowerCorner, upperCorner);

    // the upper corner need to be expanded a bit, to avoid round-off error during computation
    upperCorner += Vec3d(m_SearchRadius, m_SearchRadius, m_SearchRadius) * 0.1;

    // resize grid to fit the bounding box covering setB
    m_Grid.initialize(lowerCorner, upperCorner, m_SearchRadius);

    // clear all particle lists in each grid cell
    ParallelUtils::parallelFor(m_Grid.getAllCellData().size(),
        [&](const size_t cellIdx)
        {
            m_Grid.getCellData(cellIdx).particleIndices.resize(0);
        });

    // collect particle indices of points in setB into their corresponding cells
    ParallelUtils::parallelFor(setB.size(),
        [&](const size_t p)
        {
            auto& cellData = m_Grid.getCellData(setB[p]);
            cellData.lock.lock();
            cellData.particleIndices.push_back(p);
            cellData.lock.unlock();
        });

    // for each point in setA, collect setB neighbors within the search radius
    result.resize(setA.size());
    ParallelUtils::parallelFor(setA.size(),
        [&](const size_t p)
        {
            auto& pneighbors = result[p];

            // important: must clear the old result (if applicable)
            pneighbors.resize(0);

            const auto ppos    = setA[p];
            const auto cellIdx = m_Grid.template getCell3DIndices<int>(ppos);

            for (int k = -1; k <= 1; ++k)
            {
                int cellZ = cellIdx[2] + k;
                if (!m_Grid.template isValidCellIndex<2>(cellZ))
                {
                    continue;
                }
                for (int j = -1; j <= 1; ++j)
                {
                    int cellY = cellIdx[1] + j;
                    if (!m_Grid.template isValidCellIndex<1>(cellY))
                    {
                        continue;
                    }
                    for (int i = -1; i <= 1; ++i)
                    {
                        int cellX = cellIdx[0] + i;
                        if (!m_Grid.template isValidCellIndex<0>(cellX))
                        {
                            continue;
                        }

                        // get index q of point in setB
                        for (auto q : m_Grid.getCellData(cellX, cellY, cellZ).particleIndices)
                        {
                            const auto qpos  = setB[q];
                            const Vec3d diff = ppos - qpos;
                            const auto d2    = diff[0] * diff[0] + diff[1] * diff[1] + diff[2] * diff[2];
                            if (d2 < m_SearchRadiusSqr)
                            {
                                pneighbors.push_back(q);
                            }
                        }
                    }
                }
            }
    });
}
} // namespace imstk