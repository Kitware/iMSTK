/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkCellMesh.h"

namespace imstk
{
///
/// \class LineMesh
///
/// \brief Base class for all volume mesh types
///
class LineMesh : public CellMesh<2>
{
public:
    LineMesh() = default;
    ~LineMesh() override = default;

    int getNumLines() const { return getNumCells(); }
    void setLinesIndices(std::shared_ptr<VecDataArray<int, 2>> indices) { setCells(indices); }
    std::shared_ptr<VecDataArray<int, 2>> getLinesIndices() const { return getCells(); }

    IMSTK_TYPE_NAME(LineMesh)
};
} // namespace imstk
