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
PbdSolver::setSolverType(const PbdConstraint::SolverType& type)
{
    if (type == PbdConstraint::SolverType::GCD)
    {
        LOG(WARNING) << "GCD is NOT implemented yet, use xPBD instead";
        m_solverType = PbdConstraint::SolverType::xPBD;
        return;
    }

    m_solverType = type;
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
} // end namespace imstk
