/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkSOR.h"
#include "imstkLogger.h"
#include "imstkLinearProjectionConstraint.h"

namespace imstk
{
SOR::SOR(const SparseMatrixd& A, const Vectord& rhs) : SOR()
{
    this->setSystem(std::make_shared<LinearSystem<SparseMatrixd>>(A, rhs));
}

void
SOR::solve(Vectord& x)
{
    if (!m_linearSystem)
    {
        LOG(WARNING) << "SOR::solve: Linear system is not supplied for Gauss-Seidel solver!";
        return;
    }

    if (m_FixedLinearProjConstraints->size() == 0)
    {
        this->SORSolve(x);
    }
    else
    {
        // Do nothing for now!
    }
}

void
SOR::solve(Vectord& x, const double tolerance)
{
    this->setTolerance(tolerance);
    this->solve(x);
}

void
SOR::SORSolve(Vectord& x)
{
    const auto& b = m_linearSystem->getRHSVector();
    const auto& A = m_linearSystem->getMatrix();

    // Set the initial guess to zero
    x.setZero();

    auto   xOld    = x;
    size_t iterNum = 0;
    while (iterNum < this->getMaxNumIterations())
    {
        for (auto k = 0; k < A.outerSize(); ++k)
        {
            double diagEle   = 0.;
            double aggregate = 0.;
            for (SparseMatrixd::InnerIterator it(A, k); it; ++it)
            {
                auto col = it.col();
                if (k != col)
                {
                    aggregate += it.value() * x[col];
                }
                else
                {
                    diagEle = it.value();
                }
            }
            x[k] = (b[k] - aggregate) / diagEle; // div by zero is possible!
        }
        x *= m_relaxationFactor;
        x += (1. - m_relaxationFactor) * xOld;

        if ((x - xOld).norm() < 1.0e-4)
        {
            return;
        }
        xOld = x;
        iterNum++;
    }
}

double
SOR::getResidual(const Vectord&)
{
    return 0;
}

void
SOR::setTolerance(const double epsilon)
{
    IterativeLinearSolver::setTolerance(epsilon);
}

void
SOR::setMaxNumIterations(const size_t maxIter)
{
    IterativeLinearSolver::setMaxNumIterations(maxIter);
}

void
SOR::setSystem(std::shared_ptr<LinearSystem<SparseMatrixd>> newSystem)
{
    LinearSolver<SparseMatrixd>::setSystem(newSystem);
}

void
SOR::print() const
{
    IterativeLinearSolver::print();

    LOG(INFO) << "Solver: SOR";
    LOG(INFO) << "Tolerance: " << m_tolerance;
    LOG(INFO) << "max. iterations: " << m_maxIterations;
}
} // namespace imstk