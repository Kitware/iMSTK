/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCellMesh.h"
#include "imstkPbdConstraintContainer.h"
#include "imstkPbdMethod.h"
#include "imstkPbdObjectCellRemoval.h"
#include "imstkPbdSystem.h"

namespace imstk
{
PbdObjectCellRemoval::PbdObjectCellRemoval(std::shared_ptr<PbdMethod> pbdObj) :
    m_obj(pbdObj)
{
    // Add checks here as needed

    // Get mesh and add dummy vertex for storing removed cell
    m_mesh = std::dynamic_pointer_cast<AbstractCellMesh>(m_obj->getGeometry());
    addDummyVertex(m_mesh);

    // Update fixed node ids to account for dummy vertex at index zero
    for (int fixedId = 0; fixedId < m_obj->getPbdBody()->fixedNodeIds.size(); fixedId++)
    {
        m_obj->getPbdBody()->fixedNodeIds[fixedId]++;
    }

    // Reinitialize to account for new dummy vertex
    m_obj->initialize();

    // Note: maps on the PbdMethod are no longer valid after this point
}

void
PbdObjectCellRemoval::removeConstraints()
{
    // Mesh Data
    int       bodyId       = m_obj->getPbdBody()->bodyHandle;
    const int vertsPerCell = m_mesh->getAbstractCells()->getNumberOfComponents();
    auto      cellVerts    = std::dynamic_pointer_cast<DataArray<int>>(m_mesh->getAbstractCells()); // underlying 1D array

    // Constraint Data
    std::shared_ptr<PbdConstraintContainer>            constraintsPtr = m_obj->getPbdSystem()->getConstraints();
    const std::vector<std::shared_ptr<PbdConstraint>>& constraints    = constraintsPtr->getConstraints();

    // First process all removed cells by removing the constraints and setting the cell to the dummy vertex
    for (int i = 0; i < m_cellsToRemove.size(); i++)
    {
        int cellId = m_cellsToRemove[i];

        // Find and remove the associated constraints
        for (auto j = constraints.begin(); j != constraints.end();)
        {
            const std::vector<PbdParticleId>& vertexIds = (*j)->getParticles();

            // Dont remove any constraints that do not involve
            // every node of the cell
            if (vertexIds.size() < vertsPerCell)
            {
                j++;
                continue;
            }

            // Check that constraint involves this body and get associated vertices
            bool isBody = false;
            for (int cVertId = 0; cVertId < vertexIds.size(); cVertId++)
            {
                if (vertexIds[cVertId].first == bodyId)
                {
                    isBody = true;
                    break;
                }
            }

            // Skip if body is not involved
            if (isBody == false)
            {
                j++;
                continue;
            }

            // Check if the constraint involves ONLY the body of interest
            // This is used for removing constraints that connect two or more bodies
            bool isOnlyBody = true;
            for (int cVertId = 0; cVertId < vertexIds.size(); cVertId++)
            {
                if (vertexIds[cVertId].first != bodyId)
                {
                    isOnlyBody = false;
                    break;
                }
            }

            // Sets for comparing constraint vertices to cell vertices
            std::unordered_set<int> constraintVertIds;
            std::unordered_set<int> cellVertIds;
            // Fill in sets
            for (int vertId = 0; vertId < vertsPerCell; vertId++)
            {
                cellVertIds.insert((*cellVerts)[cellId * vertsPerCell + vertId]);
            }

            for (int cVertId = 0; cVertId < vertexIds.size(); cVertId++)
            {
                constraintVertIds.insert(vertexIds[cVertId].second);
            }

            // Check if cell nodes are a subset of the nodes used for the constraint
            bool isSubset = true;
            for (const auto& elem : constraintVertIds)
            {
                if (cellVertIds.find(elem) == cellVertIds.end())
                {
                    isSubset = false;
                    break;
                }
            }

            // Handle constraints connecting two bodies when an associated cell is deleted.
            bool isMultiBodyConstraint = false;
            if (isOnlyBody == false)
            {
                for (int cVertId = 0; cVertId < vertexIds.size(); cVertId++)
                {
                    if (vertexIds[cVertId].first == bodyId && cellVertIds.find(vertexIds[cVertId].second) != cellVertIds.end())
                    {
                        isMultiBodyConstraint = true;
                    }
                }
            }

            if (isSubset == true)
            {
                j = constraintsPtr->eraseConstraint(j);
            }
            else if (isMultiBodyConstraint && isSubset == false)
            {
                j = constraintsPtr->eraseConstraint(j);
            }
            else
            {
                j++;
            }
        }

        // Set removed cell to dummy vertex
        for (int k = 0; k < vertsPerCell; k++)
        {
            (*cellVerts)[cellId * vertsPerCell + k] = 0;
        }
    }

    if (m_cellsToRemove.size() > 0)
    {
        // Note: if the collision geometry is different from the physics geometry the collision geometry
        // will need to be updated. This is not yet implemented.
        m_mesh->getAbstractCells()->postModified();
    }
}

void
PbdObjectCellRemoval::apply()
{
    removeConstraints();
    m_removedCells.insert(m_removedCells.end(), m_cellsToRemove.begin(), m_cellsToRemove.end());
    m_cellsToRemove.clear();
}

void
PbdObjectCellRemoval::removeCellOnApply(int cellId)
{
    m_cellsToRemove.push_back(cellId);
}

void
PbdObjectCellRemoval::addDummyVertexPointSet(std::shared_ptr<PointSet> pointSet)
{
    // Add a dummy vertex to the vertices
    std::shared_ptr<VecDataArray<double, 3>> verticesPtr = pointSet->getVertexPositions();
    VecDataArray<double, 3>&                 vertices    = *verticesPtr;
    vertices.resize(vertices.size() + 1);

    for (int i = vertices.size() - 1; i >= 1; i--)
    {
        vertices[i] = vertices[i - 1];
    }

    // Note: may cause collision issues
    vertices[0] = Vec3d(0.0, 0.0, 0.0);
    pointSet->setInitialVertexPositions(std::make_shared<VecDataArray<double, 3>>(*verticesPtr));
}

void
PbdObjectCellRemoval::addDummyVertex(std::shared_ptr<AbstractCellMesh> mesh)
{
    addDummyVertexPointSet(mesh);

    // Mesh data
    const int vertsPerCell = mesh->getAbstractCells()->getNumberOfComponents();
    auto      cellVerts    = std::dynamic_pointer_cast<DataArray<int>>(mesh->getAbstractCells());

    // Then shift all indices by 1
    for (int cellId = 0; cellId < mesh->getNumCells(); cellId++)
    {
        for (int vertId = 0; vertId < vertsPerCell; vertId++)
        {
            (*cellVerts)[cellId * vertsPerCell + vertId]++;
        }
    }
}
} // namespace imstk
