/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkSpatialHashTable.h"
#include "imstkMath.h"
#include "imstkVecDataArray.h"

#include <unordered_set>

namespace imstk
{
struct PointEntry
{
    Vec3d point;
    unsigned long ID;
    double* cellSize;
};
} // namespace imstk

namespace std
{
///
/// \struct hash
/// \brief Returns a hash value for a \ref PointEntry
///
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

///
/// \struct equal_to
/// \brief Test if two points are the same or not by comparing their id and coordinates
///
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
} // namespace std

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
    void insertPoints(const VecDataArray<double, 3>& points);

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
    /// \brief Finds IDs of all points in an AABB
    /// \param result The list to contain search result
    /// \param corner1 One corner to the box
    /// \param corner2 The other corner to the box
    ///
    void getPointsInAABB(std::vector<size_t>& result, const Vec3d& corner1, const Vec3d& corner2);

    ///
    /// \brief Find IDs of all points in a sphere centered at ppos and having given radius
    /// \param pos Postision of the given point
    /// \param radius The search radius
    ///
    std::vector<size_t> getPointsInSphere(const Vec3d& ppos, double radius);

    ///
    /// \brief Find IDs of all points in a sphere centered at ppos and having given radius
    /// \param result The list to contain search result
    /// \param pos Postision of the given point
    /// \param radius The search radius
    ///
    void getPointsInSphere(std::vector<size_t>& result, const Vec3d& ppos, const double radius);

    ///
    /// \brief Clears the table
    ///
    void clear();

    ///
    /// \brief Protected constructor
    /// \param x,y,z Dimensions for each cell
    ///
    virtual void setCellSize(double x, double y, double z) override;

    ///
    /// \brief Update cell size for all points and rehash. This is called after changing the cell dimensions.
    ///
    void recomputePointHash();

protected:
    ///
    /// \brief Rehash the hash table
    ///
    virtual void rehash() override;

    float m_loadFactorMax     = 10.0f;
    unsigned long m_currentID = 0;
    std::shared_ptr<std::unordered_set<PointEntry>> m_table;
};
} // namespace imstk
