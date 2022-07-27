/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkSpatialHashTableSeparateChaining.h"

namespace imstk
{
SpatialHashTableSeparateChaining::SpatialHashTableSeparateChaining() :
    SpatialHashTable(),
    m_table(std::make_shared<std::unordered_set<PointEntry>>())
{
    this->clear();
}

void
SpatialHashTableSeparateChaining::insertPoints(const VecDataArray<double, 3>& points)
{
    for (int i = 0; i < points.size(); i++)
    {
        this->insertPoint(points[i]);
    }
}

void
SpatialHashTableSeparateChaining::insertPoint(const Vec3d& point)
{
    PointEntry entry;
    entry.point    = point;
    entry.ID       = m_currentID;
    entry.cellSize = m_cellSize;

    m_table->insert(entry);
    m_currentID++;
}

void
SpatialHashTableSeparateChaining::clear()
{
    m_table->clear();
    m_currentID = 0;
}

std::vector<size_t>
SpatialHashTableSeparateChaining::getPointsInAABB(const Vec3d& corner1, const Vec3d& corner2)
{
    std::vector<size_t> result;
    getPointsInAABB(result, corner1, corner2);
    return result;
}

void
SpatialHashTableSeparateChaining::getPointsInAABB(std::vector<size_t>& result, const Vec3d& corner1, const Vec3d& corner2)
{
    auto min_x = std::fmin(corner1.x(), corner2.x());
    auto max_x = std::fmax(corner1.x(), corner2.x());
    auto min_y = std::fmin(corner1.y(), corner2.y());
    auto max_y = std::fmax(corner1.y(), corner2.y());
    auto min_z = std::fmin(corner1.z(), corner2.z());
    auto max_z = std::fmax(corner1.z(), corner2.z());

    std::unordered_set<PointEntry> tempPoints(0);

    // Coarse iteration (false positives may exist)
    for (double x = min_x; x < max_x + m_cellSize[0]; x += m_cellSize[0])
    {
        for (double y = min_y; y < max_y + m_cellSize[1]; y += m_cellSize[1])
        {
            for (double z = min_z; z < max_z + m_cellSize[2]; z += m_cellSize[2])
            {
                PointEntry point;
                point.point    = Vec3d(x, y, z);
                point.cellSize = m_cellSize;

                auto bucket = m_table->bucket(point);

                auto first = m_table->begin(bucket);
                auto last  = m_table->end(bucket);

                for (auto p = first; p != last; ++p)
                {
                    tempPoints.insert(*p);
                }
            }
        }
    }

    // clear old data (if applicable) and allocate memory beforehand
    result.resize(0);
    result.reserve(tempPoints.size());

    // Fine iteration
    for (auto p = tempPoints.begin(); p != tempPoints.end(); ++p)
    {
        Vec3d point = p->point;
        if (point.x() >= min_x && point.x() <= max_x
            && point.y() >= min_y && point.y() <= max_y
            && point.z() >= min_z && point.z() <= max_z)
        {
            result.push_back(p->ID);
        }
    }
}

std::vector<size_t>
SpatialHashTableSeparateChaining::getPointsInSphere(const Vec3d& ppos, double radius)
{
    std::vector<size_t> result;
    getPointsInSphere(result, ppos, radius);
    return result;
}

void
SpatialHashTableSeparateChaining::getPointsInSphere(std::vector<size_t>& result, const Vec3d& ppos, const double radius)
{
    int cellSpan[3];
    for (int d = 0; d < 3; ++d)
    {
        cellSpan[d] = static_cast<int>(std::ceil(radius / m_cellSize[d]));
    }

    double                     radiusSqr = radius * radius;
    std::unordered_set<size_t> visited;
    visited.reserve(static_cast<size_t>(cellSpan[0] * cellSpan[1] * cellSpan[2]));

    // clear the old result (if applicable)
    result.resize(0);

    for (int i = -cellSpan[0]; i <= cellSpan[0]; ++i)
    {
        for (int j = -cellSpan[1]; j <= cellSpan[1]; ++j)
        {
            for (int k = -cellSpan[2]; k <= cellSpan[2]; ++k)
            {
                PointEntry point;
                point.point = Vec3d(ppos[0] + m_cellSize[0] * i,
                        ppos[1] + m_cellSize[1] * j,
                        ppos[2] + m_cellSize[2] * k);
                point.cellSize = m_cellSize;

                auto bucket = m_table->bucket(point);

                // avoid visiting a bucket more than once
                // (that happens due to numerical round-off)
                if (visited.find(bucket) != visited.end())
                {
                    continue;
                }
                visited.insert(bucket);

                auto first = m_table->begin(bucket);
                auto last  = m_table->end(bucket);

                for (auto it = first; it != last; ++it)
                {
                    const Vec3d& qpos = it->point;
                    const Vec3d  diff = ppos - qpos;
                    const auto   d2   = diff[0] * diff[0] + diff[1] * diff[1] + diff[2] * diff[2];
                    if (d2 < radiusSqr)
                    {
                        result.push_back(it->ID);
                    }
                }
            }
        }
    }
}

void
SpatialHashTableSeparateChaining::setLoadFactorMax(float loadFactorMax)
{
    m_loadFactorMax = loadFactorMax;
    m_table->max_load_factor(m_loadFactorMax);
    m_table->rehash(m_table->bucket_count());
}

void
SpatialHashTableSeparateChaining::setCellSize(double x, double y, double z)
{
    m_cellSize[0] = x;
    m_cellSize[1] = y;
    m_cellSize[2] = z;

    recomputePointHash();
}

void
SpatialHashTableSeparateChaining::recomputePointHash()
{
    // copy points from the hash table to a vector, then clear the table
    std::vector<PointEntry> points;
    points.reserve(m_table->size());
    points.insert(points.end(), m_table->begin(), m_table->end());
    m_table->clear();

    for (auto& point : points)
    {
        point.cellSize = m_cellSize;
    }

    // insert points back to the table
    for (auto& point : points)
    {
        m_table->insert(point);
    }
}

void
SpatialHashTableSeparateChaining::rehash()
{
    m_table->rehash(m_table->bucket_count());
}
} // namespace imstk
