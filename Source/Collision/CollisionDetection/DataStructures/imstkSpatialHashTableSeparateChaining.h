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

#ifndef imstkSpatialHashTableSeparateChaining_h
#define imstkSpatialHashTableSeparateChaining_h

#include <memory>
#include <unordered_set>

#include "imstkMath.h"
#include "imstkSpatialHashTable.h"

namespace imstk
{
struct PointEntry
{
    Vec3d point;
    unsigned long ID;
    double * cellSize;
};
}

namespace std
{
template<> struct hash<imstk::PointEntry>
{
    size_t operator()(const imstk::PointEntry& point) const
    {
        unsigned int x = (unsigned int)(point.point.x() / point.cellSize[0]);
        unsigned int y = (unsigned int)(point.point.y() / point.cellSize[1]);
        unsigned int z = (unsigned int)(point.point.z() / point.cellSize[2]);

        return (104729 * x + 104743 * y + 104759 * z);
    }
};

template<> struct equal_to<imstk::PointEntry>
{
    size_t operator()(const imstk::PointEntry& point1, const imstk::PointEntry& point2) const
    {
        if (point1.ID != point2.ID)
        {
            return false;
        }

        if (point1.point != point2.point)
        {
            return false;
        }

        return true;
    }
};
}

namespace imstk
{
///
/// \class SpatialHashTableSeparateChaining
///
/// \brief Implementation of SpatialHashTable using separate chaining
///
class SpatialHashTableSeparateChaining : public SpatialHashTable
{
public:
    ///
    /// \brief Default constructor
    ///
    SpatialHashTableSeparateChaining();

    ///
    /// \brief Insert an array of points
    /// \param points An array of point
    ///
    void insertPoints(const StdVectorOfVec3d& points);

    ///
    /// \brief Insert an array of points
    /// \param point A point
    ///
    void insertPoint(const Vec3d& point);

    ///
    /// \brief Sets the max load factor
    /// \param loadFactorMax The new capacity after a rehash
    ///
    void setLoadFactorMax(float loadFactorMax);

    ///
    /// \brief Finds IDs of all points in an AABB
    /// \param corner1 One corner to the box
    /// \param corner2 The other corner to the box
    ///
    std::vector<size_t> getPointsInAABB(const Vec3d& corner1, const Vec3d& corner2);

    ///
    /// \brief Clears the table
    ///
    void clear();

    ///
    /// \brief Protected constructor
    /// \param x,y,z Dimensions for each cell
    ///
    virtual void setCellSize(double x, double y, double z) override;

protected:
    ///
    /// \brief Rehash the hash table
    ///
    virtual void rehash() override;

    float m_loadFactorMax = 10.0f;

    unsigned long m_currentID = 0;

    std::shared_ptr<std::unordered_set<PointEntry>> m_table;
};
}

#endif
