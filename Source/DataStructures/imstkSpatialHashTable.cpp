/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkSpatialHashTable.h"

namespace imstk
{
SpatialHashTable::SpatialHashTable()
{
    m_cellSize[0] = 0.1;
    m_cellSize[1] = 0.1;
    m_cellSize[2] = 0.1;
}
} // namespace imstk
