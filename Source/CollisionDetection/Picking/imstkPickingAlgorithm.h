/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkGeometryAlgorithm.h"
#include "imstkCollisionData.h"

namespace imstk
{
///
/// \brief PickData provides ids to indicate what was picked
/// These may be optionally used to indicate the selection
///
struct PickData
{
    int ids[4]  = { -1, -1, -1, -1 };    ///< Ids of the cell or vertices
    int idCount = 0;                     ///< Indicates number of vertices (if 1 a cell or individual vertex)
    CellTypeId cellType = IMSTK_VERTEX;  ///< Indicates picked cell type
    Vec3d pickPoint     = Vec3d::Zero(); ///< Some pickings may produce specific points on an element
};

///
/// \class PickingAlgorithm
///
/// \brief Abstract functor for picking/selecting of geometry.
/// Differs from collision in that the picking criteria can follow
/// different logic (ray, nearest, various exclusion criteria, etc).
/// Some pickers utilize CollisionDetectionAlgorithms for intersection tests.
///
/// \note: A VTK backend for these might be nice too (octree accelerated
/// and all the cell cases worked out)
/// \note: A factory mapping shape types to picking types is
/// a strong possibility.
///
class PickingAlgorithm : public GeometryAlgorithm
{
public:
    PickingAlgorithm()
    {
        setNumInputPorts(1);
        setNumOutputPorts(0);
    }

    ~PickingAlgorithm() override = default;

    ///
    /// \brief Perform picking on provided geometry
    ///
    const std::vector<PickData>& pick(std::shared_ptr<Geometry> geomToPick)
    {
        setInput(geomToPick, 0);
        m_results.clear();
        update();
        return m_results;
    }

    const std::vector<PickData>& getPickData() const { return m_results; }

protected:
    std::vector<PickData> m_results;
};
} // namespace imstk