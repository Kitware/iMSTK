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

namespace imstk
{
PbdSolver::PbdSolver() :
    m_partitionedConstraints(std::make_shared<std::vector<PBDConstraintVector>>()),
    m_constraints(std::make_shared<PBDConstraintVector>()),
    m_positions(std::make_shared<StdVectorOfVec3d>()),
    m_invMasses(std::make_shared<StdVectorOfReal>())
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
    StdVectorOfVec3d&      currPositions = *m_positions;
    const StdVectorOfReal& invMasses     = *m_invMasses;

    const PBDConstraintVector&              constraints = *m_constraints;
    const std::vector<PBDConstraintVector>& partitionedConstraints = *m_partitionedConstraints;

    // zero out the Lagrange multiplier
    for (size_t j = 0; j < constraints.size(); ++j)
    {
        constraints[j]->zeroOutLambda();
    }

    for (size_t j = 0; j < partitionedConstraints.size(); j++)
    {
        const PBDConstraintVector& constraintPartition = partitionedConstraints[j];
        ParallelUtils::parallelFor(constraintPartition.size(),
            [&](const size_t idx) { constraintPartition[idx]->zeroOutLambda(); }
            );
    }

    unsigned int i = 0;
    while (i++ < m_iterations)
    {
        for (size_t j = 0; j < constraints.size(); ++j)
        {
            constraints[j]->projectConstraint(invMasses, m_dt, m_solverType, currPositions);
        }

        for (size_t j = 0; j < partitionedConstraints.size(); j++)
        {
            const PBDConstraintVector& constraintPartition = partitionedConstraints[j];

            ParallelUtils::parallelFor(constraintPartition.size(),
                [&](const size_t idx)
                {
                    constraintPartition[idx]->projectConstraint(invMasses, m_dt, m_solverType, currPositions);
                });
            // Sequential
            /*for (size_t k = 0; k < constraintPartition.size(); k++)
            {
                constraintPartition[k]->solvePositionConstraint(currPositions, invMasses);
            }*/
        }
    }
}

PbdCollisionSolver::PbdCollisionSolver() :
    m_collisionConstraints(std::make_shared<std::list<PBDCollisionConstraintVector*>>()),
    m_collisionConstraintsData(std::make_shared<std::list<CollisionConstraintData>>())
{
}

void
PbdCollisionSolver::addCollisionConstraints(PBDCollisionConstraintVector* constraints,
                                            std::shared_ptr<StdVectorOfVec3d> posA, std::shared_ptr<StdVectorOfReal> invMassA,
                                            std::shared_ptr<StdVectorOfVec3d> posB, std::shared_ptr<StdVectorOfReal> invMassB)
{
    m_collisionConstraints->push_back(constraints);
    m_collisionConstraintsData->push_back({ posA, invMassA, posB, invMassB });
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
            std::list<CollisionConstraintData>::iterator colDataIter = m_collisionConstraintsData->begin();
            for (auto constraintList : *m_collisionConstraints)
            {
                CollisionConstraintData             colData     = *colDataIter;
                const PBDCollisionConstraintVector& constraints = *constraintList;
                for (size_t j = 0; j < constraints.size(); j++)
                {
                    constraints[j]->projectConstraint(colData.m_invMassA.get(), colData.m_invMassB.get(), colData.m_posA.get(), colData.m_posB.get());
                }
                colDataIter++;
            }
        }
        m_collisionConstraints->clear();
        m_collisionConstraintsData->clear();
    }
}
} // end namespace imstk
