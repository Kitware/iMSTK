/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/
#pragma once

#include "imstkSceneObject.h"
#include "imstkMacros.h"

#include <unordered_set>

namespace imstk
{
class PointSet;
class AbstractCellMesh;
class PbdObject;
///
/// \class PbdObjectCellRemoval
///
/// \brief This class defines a method of removing cells and their associated constraints from a
/// Pbd object.
///
class PbdObjectCellRemoval : public SceneObject
{
public:
    PbdObjectCellRemoval(std::shared_ptr<PbdObject> pbdObj);
    ~PbdObjectCellRemoval() override = default;

    IMSTK_TYPE_NAME(PbdObjectCellRemoval)

    ///
    /// \brief Adds cell to list of cells to be removed
    ///
    void removeCellOnApply(int cellId);

    ///
    /// \brief removed cells and associated constraints
    ///
    void apply();

    ///
    /// \brief Get ids of cells that have been removed
    ///
    std::vector<int> getRemovedCells() { return m_removedCells; }

protected:

    void removeConstraints();

    void addDummyVertexPointSet(std::shared_ptr<PointSet> pointSet);
    void addDummyVertex(std::shared_ptr<AbstractCellMesh> mesh);

    std::shared_ptr<PbdObject> m_obj;         ///< Object that cells are removed from
    std::shared_ptr<AbstractCellMesh> m_mesh; ///< Mesh from object cells are removed from
    std::vector<int> m_cellsToRemove;         ///< List of cells to remove, cleared after removal
    std::vector<int> m_removedCells;          ///< Cells that have been removed
};
} // namespace imstk