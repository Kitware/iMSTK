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

#include "imstkIterativeLinearSolver.h"
#include "imstkLogger.h"
#include "imstkMacros.h"

namespace imstk
{
void
IterativeLinearSolver::setMaxNumIterations(const size_t maxIter)
{
    m_maxIterations = maxIter;
}

size_t
IterativeLinearSolver::getMaxNumIterations() const
{
    return m_maxIterations;
}

const Vectord&
IterativeLinearSolver::getResidualVector()
{
    return m_residual;
}

const Vectord&
IterativeLinearSolver::getResidualVector(const Vectord& x)
{
    m_linearSystem->computeResidual(x, this->m_residual);
    return m_residual;
}

double
IterativeLinearSolver::getResidual(const Vectord& x)
{
    m_linearSystem->computeResidual(x, this->m_residual);
    return m_residual.squaredNorm();
}

void
IterativeLinearSolver::print() const
{
    // Print Type
    LinearSolver::print();

    LOG(INFO) << "Solver type (direct/iterative): Iterative";
}

void
IterativeLinearSolver::solve(Vectord& imstkNotUsed(x))
{
    LOG(WARNING) << "IterativeLinearSolver::solve is not implemented!";
    /*
    if (!m_linearSystem)
    {
        LOG(WARNING) << "The linear system should be assigned before solving!";
        return;
    }

    auto epsilon = m_tolerance * m_tolerance;
    m_linearSystem->computeResidual(x, m_residual);

    for (size_t i = 0; i < m_maxIterations; ++i)
    {
        if (m_residual.squaredNorm() < epsilon)
        {
            return;
        }

        this->iterate(x);
    }
    */
}
}