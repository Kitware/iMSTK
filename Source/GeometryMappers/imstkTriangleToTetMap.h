/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkPointwiseMap.h"
#include "imstkMath.h"
#include "imstkTypes.h"

namespace imstk
{
///
/// \class TriangleToTetMap
///
/// \brief SurfaceToTetrahedralMap serves as a PointwiseMap but also maps
/// tets to triangle faces.
///
class TriangleToTetMap : public PointwiseMap
{
public:
    TriangleToTetMap();
    TriangleToTetMap(
        std::shared_ptr<Geometry> parent,
        std::shared_ptr<Geometry> child);
    ~TriangleToTetMap() override = default;

    IMSTK_TYPE_NAME(TriangleToTetMap)

    ///
    /// \brief Compute the map
    ///
    void compute() override;

    ///
    /// \brief Compute tet vertex id to surf vertex id map
    ///
    void computeTriToTetMap(std::unordered_map<int, int>& triToTetMap);

    ///
    /// \brief Get the tet id that contains the triangle
    ///
    int getParentTetId(const int triId) const;

public:
    std::unordered_map<int, int> m_triToTetMap;
};
} // namespace imstk