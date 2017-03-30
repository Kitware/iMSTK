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

#include "imstkSpatialHashTableSeparateChaining.h"

namespace imstk
{

SpatialHashTableSeparateChaining::SpatialHashTableSeparateChaining()
    : SpatialHashTable()
{
    m_table = std::make_shared<std::unordered_set<PointEntry>>();
    this->clear();
}

void
SpatialHashTableSeparateChaining::insertPoints(const StdVectorOfVec3d& points)
{
    // TODO: make more efficient
    for (auto i = 0; i < points.size(); i++)
    {
        this->insertPoint(points[i]);
    }
}

void
SpatialHashTableSeparateChaining::insertPoint(const Vec3d& point)
{
    PointEntry entry;
    entry.point = point;
    entry.ID = m_currentID;
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
    auto min_x = std::fmin(corner1.x(), corner2.x());
    auto max_x = std::fmax(corner1.x(), corner2.x());
    auto min_y = std::fmin(corner1.y(), corner2.y());
    auto max_y = std::fmax(corner1.y(), corner2.y());
    auto min_z = std::fmin(corner1.z(), corner2.z());
    auto max_z = std::fmax(corner1.z(), corner2.z());

    std::unordered_set<PointEntry> tempPoints(0);
    
    // Coarse iteration (false positives may exist)
    for (double x = min_x; x < max_x + m_cellSize[0]; x += m_cellSize[0])
    for (double y = min_y; y < max_y + m_cellSize[1]; y += m_cellSize[1])
    for (double z = min_z; z < max_z + m_cellSize[2]; z += m_cellSize[2])
    {
        PointEntry point;
        point.point = Vec3d(x, y, z);
        point.cellSize = m_cellSize;

        auto bucket = m_table->bucket(point);

        auto first = m_table->begin(bucket);
        auto last = m_table->end(bucket);

        for (auto p = first; p != last; ++p)
        {
            tempPoints.insert(*p);
        }
    }

    // Allocate beforehand
    std::vector<size_t> points(0);
    points.reserve(tempPoints.size());

    // Fine iteration
    for (auto p = tempPoints.begin(); p != tempPoints.end(); ++p)
    {
        Vec3d point = p->point;
        if (point.x() >= min_x && point.x() <= max_x &&
            point.y() >= min_y && point.y() <= max_y &&
            point.z() >= min_z && point.z() <= max_z)
        {
            points.push_back(p->ID);
        }
    }

    return points;
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

    this->rehash();
}

void
SpatialHashTableSeparateChaining::rehash()
{
    m_table->rehash(m_table->bucket_count());
}

}

