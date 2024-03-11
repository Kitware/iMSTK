/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkAbstractCellMesh.h"
#include "imstkBurnable.h"
#include "imstkCellMesh.h"
#include "imstkParallelUtils.h"
#include "imstkPbdConstraint.h"
#include "imstkPbdConstraintContainer.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCellRemoval.h"
#include "imstkPbdSolver.h"
#include "imstkTaskGraph.h"
#include "imstkTaskNode.h"

namespace imstk
{
Burnable::Burnable(const std::string& name) : SceneBehaviour(false, name)
{
}

void
Burnable::init()
{
    m_burnableObject = std::dynamic_pointer_cast<PbdObject>(getEntity().lock());

    CHECK(m_burnableObject != nullptr) << "Burnable requires a input PBD object,"
        "please add it on creation";

    if (!m_trackOnly)
    {
        // Create cell remover for removing torn cells
        m_cellRemover = std::make_shared<PbdObjectCellRemoval>(m_burnableObject, m_updateType);
    }

    // Allocate memory for mesh state and initialize values
    auto cellMesh = std::dynamic_pointer_cast<AbstractCellMesh>(m_burnableObject->getPhysicsGeometry());
    int  numCells = cellMesh->getNumCells();

    m_burnDamagePtr = std::make_shared<DataArray<double>>(numCells);
    m_burnVisualPtr = std::make_shared<DataArray<double>>(numCells);
    // Initialize to zero
    for (int i = 0; i < numCells; i++)
    {
        m_burnDamagePtr->at(i) = 0.0;
        m_burnVisualPtr->at(i) = 0.0;
    }

    cellMesh->setCellAttribute("BurnDamage", m_burnDamagePtr);
    cellMesh->setCellAttribute("BurnVisual", m_burnVisualPtr);
}

// Check state of mesh and delete cells that are burned away
void
Burnable::visualUpdate(const double& dt)
{
    if (m_trackOnly)
    {
        return;
    }

    // Check that the cellConstraintMap exists, if not make it
    if (m_burnableObject->getPbdBody()->cellConstraintMap.empty())
    {
        m_burnableObject->computeCellConstraintMap();
    }

    // Get body id
    auto pbdBody = m_burnableObject->getPbdBody();

    // Mesh data
    auto      cellMesh     = std::dynamic_pointer_cast<AbstractCellMesh>(m_burnableObject->getPhysicsGeometry());
    auto      cellVerts    = std::dynamic_pointer_cast<DataArray<int>>(cellMesh->getAbstractCells()); // underlying 1D array
    const int vertsPerCell = cellMesh->getAbstractCells()->getNumberOfComponents();

    // Mesh state data
    auto               burnStatePtr = std::dynamic_pointer_cast<DataArray<double>>(cellMesh->getCellAttribute("BurnDamage"));
    DataArray<double>& burnState    = *burnStatePtr;

    // Check if the cell is burned and remove if so
    ParallelUtils::SpinLock lock;
    ParallelUtils::parallelFor(cellMesh->getNumCells(),
        [&](const int cellId)
        {
            if (burnState[cellId] >= 1.0)
            {
                lock.lock();
                m_cellRemover->removeCellOnApply(cellId);
                pbdBody->cellConstraintMap.erase(cellId);
                burnState[cellId] = 0.0;
                lock.unlock();
            }
        }, cellMesh->getNumCells() > 50);

    m_cellRemover->apply();
}
} // namespace imstk