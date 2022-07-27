/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkDirectLinearSolver.h"
#include "imstkLogger.h"

namespace imstk
{
DirectLinearSolver<Matrixd>::
DirectLinearSolver(const Matrixd& matrix, const Vectord& b)
{
    m_linearSystem = std::make_shared<LinearSystem<Matrixd>>(matrix, b);
    m_solver.compute(matrix);
}

void
DirectLinearSolver<Matrixd>::
setSystem(std::shared_ptr<LinearSystem<Matrixd>> newSystem)
{
    LinearSolver<Matrixd>::setSystem(newSystem);
    m_solver.compute(m_linearSystem->getMatrix());
}

DirectLinearSolver<SparseMatrixd>::
DirectLinearSolver(const SparseMatrixd& matrix, const Vectord& b)
{
    m_linearSystem = std::make_shared<LinearSystem<SparseMatrixd>>(matrix, b);
    m_solver.compute(matrix);
}

void
DirectLinearSolver<SparseMatrixd>::
setSystem(std::shared_ptr<LinearSystem<SparseMatrixd>> newSystem)
{
    LinearSolver<SparseMatrixd>::setSystem(newSystem);
    m_solver.compute(m_linearSystem->getMatrix());
}

void
DirectLinearSolver<SparseMatrixd>::solve(const Vectord& rhs, Vectord& x)
{
    if (!m_linearSystem)
    {
        LOG(FATAL) << "Linear system has not been set";
    }
    x = m_solver.solve(rhs);
}

void
DirectLinearSolver<SparseMatrixd>::solve(Vectord& x)
{
    if (!m_linearSystem)
    {
        LOG(FATAL) << "Linear system has not been set";
    }
    x.setZero();

    auto b = m_linearSystem->getRHSVector();
    x = m_solver.solve(b);
}

void
DirectLinearSolver<Matrixd>::solve(const Vectord& rhs, Vectord& x)
{
    if (!m_linearSystem)
    {
        LOG(FATAL) << "Linear system has not been set";
    }
    x = m_solver.solve(rhs);
}

void
DirectLinearSolver<Matrixd>::solve(Vectord& x)
{
    if (!m_linearSystem)
    {
        LOG(FATAL) << "Linear system has not been set";
    }
    x.setZero();

    auto b = m_linearSystem->getRHSVector();
    x = m_solver.solve(b);
}
} // namespace imstk