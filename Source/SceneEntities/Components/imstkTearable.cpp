/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCellMesh.h"
#include "imstkParallelUtils.h"
#include "imstkPbdConstraint.h"
#include "imstkPbdConstraintContainer.h"
#include "imstkPbdMethod.h"
#include "imstkPbdObjectCellRemoval.h"
#include "imstkPbdSolver.h"
#include "imstkPbdSystem.h"
#include "imstkTaskGraph.h"
#include "imstkTaskNode.h"
#include "imstkTearable.h"

namespace imstk
{
Tearable::Tearable(const std::string& name) : SceneBehaviour(true, name)
{
    m_tearableHandleNode = std::make_shared<TaskNode>([this]()
        {
            handleTearable();
    }, "TearableHandle");
}

void
Tearable::init()
{
    auto entity = getEntity().lock();
    CHECK(entity != nullptr) << "Cannot acquire entity";
    m_tearableObject = entity->getComponent<PbdMethod>();

    CHECK(m_tearableObject != nullptr) << "Tearable requires an input PbdMethod,"
        "please add it on creation";

    // Create cell remover for removing torn cells
    m_cellRemover = std::make_shared<PbdObjectCellRemoval>(m_tearableObject);

    // Add task nodes
    m_taskGraph->addNode(m_tearableHandleNode);
    m_taskGraph->addNode(m_tearableObject->getPbdSystem()->getUpdateVelocityNode());
    m_taskGraph->addNode(m_tearableObject->getPbdSystem()->getTaskGraph()->getSink());
}

void
Tearable::handleTearable()
{
    // Check that the cellConstraintMap exists, if not make it
    if (m_tearableObject->getPbdBody()->cellConstraintMap.empty())
    {
        m_tearableObject->computeCellConstraintMap();
    }

    // Get body id
    auto pbdBody = m_tearableObject->getPbdBody();
    int  bodyId  = m_tearableObject->getPbdBody()->bodyHandle;

    // Mesh data
    auto cellMesh = std::dynamic_pointer_cast<AbstractCellMesh>(m_tearableObject->getGeometry());
    // auto      cellVerts    = std::dynamic_pointer_cast<DataArray<int>>(cellMesh->getAbstractCells()); // underlying 1D array
    // const int vertsPerCell = cellMesh->getAbstractCells()->getNumberOfComponents();

    // Check the strain state of the cell and remove if strain is greater than max strain
    ParallelUtils::SpinLock lock;
    ParallelUtils::parallelFor(cellMesh->getNumCells(),
        [&](const int cellId)
        {
            auto& constraints = m_tearableObject->getCellConstraints(cellId);

            bool remove = false;
            for (int constraintId = 0; constraintId < constraints.size(); constraintId++)
            {
                // check that constraint only involves this body
                const std::vector<PbdParticleId>& cVertexIds = constraints[constraintId]->getParticles(); ///< Vertices that are part of the constraint

                // Check that constraint involves this body and get associated vertices
                bool isBody = true;
                for (int cVertId = 0; cVertId < cVertexIds.size(); cVertId++)
                {
                    if (cVertexIds[cVertId].first != bodyId)
                    {
                        isBody = false;
                    }
                }

                double strain        = 0.0;
                double constraintC   = constraints[constraintId]->getConstraintC();
                double constraintRef = constraints[constraintId]->getRestValue();

                // Some constraints have a reference state of 0, dividing by zero is bad(TM) so
                // use constraint value without normalizing to a strain like measure (length/length)
                if (fabs(constraintRef) <= 1E-7)
                {
                    strain = constraintC;
                }
                else
                {
                    strain = constraintC / constraintRef;
                }

                if (strain > m_maxStrain)
                {
                    remove = true;
                    break;
                }
            }
            if (remove == true)
            {
                lock.lock();
                m_cellRemover->removeCellOnApply(cellId);
                pbdBody->cellConstraintMap.erase(cellId);
                lock.unlock();
            }
        }, cellMesh->getNumCells() > 50);

    m_cellRemover->apply();
}

void
Tearable::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    // Add the cell removal check after the constraints have all been solved
    m_taskGraph->addEdge(source, m_tearableObject->getPbdSystem()->getUpdateVelocityNode());
    m_taskGraph->addEdge(m_tearableObject->getPbdSystem()->getUpdateVelocityNode(), m_tearableHandleNode);
    m_taskGraph->addEdge(m_tearableHandleNode, m_tearableObject->getPbdSystem()->getTaskGraph()->getSink());
    m_taskGraph->addEdge(m_tearableObject->getPbdSystem()->getTaskGraph()->getSink(), sink);
}
} // namespace imstk