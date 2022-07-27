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
    m_dt(0.0),
    m_constraints(std::make_shared<PbdConstraintContainer>()),
    m_positions(std::make_shared<VecDataArray<double, 3>>()),
    m_invMasses(std::make_shared<DataArray<double>>())
{
}

void
PbdSolver::solve()
{
    // Solve the constraints and partitioned constraints
    VecDataArray<double, 3>& currPositions = *m_positions;
    const DataArray<double>& invMasses     = *m_invMasses;

    const std::vector<std::shared_ptr<PbdConstraint>>&              constraints = m_constraints->getConstraints();
    const std::vector<std::vector<std::shared_ptr<PbdConstraint>>>& partitionedConstraints = m_constraints->getPartitionedConstraints();

    // zero out the Lagrange multiplier
    for (const auto& constraint : constraints)
    {
        constraint->zeroOutLambda();
    }

    for (const auto& constraintPartition : partitionedConstraints)
    {
        ParallelUtils::parallelFor(constraintPartition.size(),
            [&](const size_t idx)
            {
                constraintPartition[idx]->zeroOutLambda();
            });
    }

    unsigned int i = 0;
    while (i++ < m_iterations)
    {
        for (const auto& constraint : constraints)
        {
            constraint->projectConstraint(invMasses, m_dt, m_solverType, currPositions);
        }

        for (const auto& constraintPartition : partitionedConstraints)
        {
            ParallelUtils::parallelFor(constraintPartition.size(),
                [&](const size_t idx)
                {
                    constraintPartition[idx]->projectConstraint(invMasses, m_dt, m_solverType, currPositions);
                });
            //// Sequential
            //for (size_t k = 0; k < constraintPartition.size(); k++)
            //{
            //    constraintPartition[k]->projectConstraint(invMasses, m_dt, m_solverType, currPositions);
            //}
        }
    }
}

PbdCollisionSolver::PbdCollisionSolver() :
    m_collisionConstraints(std::make_shared<std::list<std::vector<PbdCollisionConstraint*>*>>())
{
}

void
PbdCollisionSolver::addCollisionConstraints(std::vector<PbdCollisionConstraint*>* constraints)
{
    m_collisionConstraints->push_back(constraints);
}

void
PbdCollisionSolver::solve()
{
    // Solve collision constraints
    if (m_collisionConstraints->size() > 0)
    {
        unsigned int i = 0;
        while (i++ < m_collisionIterations)
        {
            for (auto constraintList : *m_collisionConstraints)
            {
                const std::vector<PbdCollisionConstraint*>& constraints = *constraintList;
                for (size_t j = 0; j < constraints.size(); j++)
                {
                    constraints[j]->solvePosition();
                }
            }
        }

        m_collisionConstraints->clear();
    }
}
} // namespace imstk