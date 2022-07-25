/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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
} // namespace imstk