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

#pragma once

#include "imstkGeometryAlgorithm.h"
#include "imstkCollisionData.h"

namespace imstk
{
///
/// \brief PickData provides a set of ids, an idCount, & a cell type
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