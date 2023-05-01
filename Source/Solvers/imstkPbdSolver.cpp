/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdSolver.h"
#include "imstkLogger.h"
#include "imstkParallelUtils.h"
#include "imstkPbdCollisionConstraint.h"
#include "imstkPbdConstraintContainer.h"

namespace imstk
{
PbdSolver::PbdSolver() :
    m_constraints(std::make_shared<PbdConstraintContainer>()),
    m_constraintLists(std::make_shared<std::list<std::vector<PbdConstraint*>*>>())
{
}

void
PbdSolver::solve()
{
    const std::vector<std::shared_ptr<PbdConstraint>>&              constraints = m_constraints->getConstraints();
    const std::vector<std::vector<std::shared_ptr<PbdConstraint>>>& partitionedConstraints = m_constraints->getPartitionedConstraints();

    // Zero out the Lagrange multiplier
    for (const auto& constraint : constraints)
    {
        constraint->zeroOutLambda();
    }

    // Zero out paritioned constraints
    for (const auto& constraintPartition : partitionedConstraints)
    {
        ParallelUtils::parallelFor(constraintPartition.size(),
            [&](const size_t idx)
            {
                constraintPartition[idx]->zeroOutLambda();
            });
    }

    // Zero out insertion/collision constraints
    for (auto constraintList : *m_constraintLists)
    {
        const std::vector<PbdConstraint*>& constraintVec = *constraintList;
        for (size_t j = 0; j < constraintVec.size(); j++)
        {
            constraintVec[j]->zeroOutLambda();
        }
    }

    unsigned int i = 0;
    while (i++ < m_iterations)
    {
        // Project collision and all external constraints
        for (auto constraintList : *m_constraintLists)
        {
            const std::vector<PbdConstraint*>& constraintVec = *constraintList;
            for (size_t j = 0; j < constraintVec.size(); j++)
            {
                constraintVec[j]->projectConstraint(*m_state, m_dt, m_solverType);
            }
        }

        // Project all internal body constraints
        for (const auto& constraint : constraints)
        {
            constraint->projectConstraint(*m_state, m_dt, m_solverType);
        }

        for (const auto& constraintPartition : partitionedConstraints)
        {
            ParallelUtils::parallelFor(constraintPartition.size(),
                [&](const size_t idx)
                {
                    constraintPartition[idx]->projectConstraint(*m_state, m_dt, m_solverType);
                });
            //// Sequential
            //for (size_t k = 0; k < constraintPartition.size(); k++)
            //{
            //    constraintPartition[k]->projectConstraint(invMasses, m_dt, m_solverType, currPositions);
            //}
        }
    }
}
} // namespace imstk