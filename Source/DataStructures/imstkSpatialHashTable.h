/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

namespace imstk
{
///
/// \class SpatialHashTable
///
/// \brief Abstract class for spatial hash tables
///
class SpatialHashTable
{
public:
    ///
    /// \brief Protected constructor
    /// \param x,y,z Dimensions for each cell
    ///
    virtual void setCellSize(double x, double y, double z) = 0;

protected:
    ///
    /// \brief Rehash the hash table
    ///
    virtual void rehash() = 0;

    ///
    /// \brief Protected constructor
    ///
    SpatialHashTable();

    double m_cellSize[3];
};
} // namespace imstk
