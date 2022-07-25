/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkConjugateGradient.h"
#include "imstkLinearProjectionConstraint.h"
#include "imstkLogger.h"

namespace imstk
{
ConjugateGradient::ConjugateGradient()
{
    m_type = Type::ConjugateGradient;
    m_cgSolver.setMaxIterations(m_maxIterations);
    m_cgSolver.setTolerance(m_tolerance);
}

ConjugateGradient::ConjugateGradient(const SparseMatrixd& A, const Vectord& rhs) : ConjugateGradient()
{
    this->setSystem(std::make_shared<LinearSystem<SparseMatrixd>>(A, rhs));
}

void
ConjugateGradient::applyLinearProjectionFilter(Vectord& x, const std::vector<LinearProjectionConstraint>& linProj, const bool setVal)
{
    for (auto& localProjector : linProj)
    {
        const auto threeI = 3 * localProjector.getNodeId();
        Vec3d      p      = localProjector.getProjector() * Vec3d(x(threeI), x(threeI + 1), x(threeI + 2));

        if (setVal)
        {
            p += (Mat3d::Identity() - localProjector.getProjector()) * localProjector.getValue();
        }

        x(threeI)     = p.x();
        x(threeI + 1) = p.y();
        x(threeI + 2) = p.z();
    }
}

void
ConjugateGradient::solve(Vectord& x)
{
    if (!m_linearSystem)
    {
        LOG(WARNING) << "Linear system is not supplied for CG solver!";
        return;
    }

    if (!(m_FixedLinearProjConstraints || m_DynamicLinearProjConstraints))
    {
        x = m_cgSolver.solve(m_linearSystem->getRHSVector());
    }
    else
    {
        this->modifiedCGSolve(x);
    }
}

void
ConjugateGradient::modifiedCGSolve(Vectord& x)
{
    const auto& b = m_linearSystem->getRHSVector();
    const auto& A = m_linearSystem->getMatrix();

    // Set the initial guess to zero
    x.setZero();
    if (m_DynamicLinearProjConstraints)
    {
        applyLinearProjectionFilter(x, *m_DynamicLinearProjConstraints, true);
    }

    if (m_FixedLinearProjConstraints)
    {
        applyLinearProjectionFilter(x, *m_FixedLinearProjConstraints, true);
    }

    auto res = b;
    if (m_DynamicLinearProjConstraints)
    {
        applyLinearProjectionFilter(res, *m_DynamicLinearProjConstraints, false);
    }
    if (m_FixedLinearProjConstraints)
    {
        applyLinearProjectionFilter(res, *m_FixedLinearProjConstraints, false);
    }
    auto       c     = res;
    auto       delta = res.dot(res);
    const auto eps   = m_tolerance * m_tolerance * delta;
    double     alpha = 0.0;
    Vectord    q;
    size_t     iterNum = 0;

    while (delta > eps)
    {
        q = A * c;
        if (m_DynamicLinearProjConstraints)
        {
            applyLinearProjectionFilter(q, *m_DynamicLinearProjConstraints, false);
        }
        if (m_FixedLinearProjConstraints)
        {
            applyLinearProjectionFilter(q, *m_FixedLinearProjConstraints, false);
        }
        double dotval = c.dot(q);
        if (dotval != 0.0)
        {
            alpha = delta / dotval;
        }
        else
        {
            LOG(WARNING) << "Warning: denominator zero. Terminating MCG iteration!";
            return;
        }
        x   += alpha * c;
        res -= alpha * q;
        const double deltaPrev = delta;
        delta = res.dot(res);
        c    *= delta / deltaPrev;
        c    += res;
        if (m_DynamicLinearProjConstraints)
        {
            applyLinearProjectionFilter(c, *m_DynamicLinearProjConstraints, false);
        }
        if (m_FixedLinearProjConstraints)
        {
            applyLinearProjectionFilter(c, *m_FixedLinearProjConstraints, false);
        }

        if (++iterNum >= m_maxIterations)
        {
            //LOG(WARNING) << "ConjugateGradient::modifiedCGSolve - The solver did not converge after max. iterations";
            break;
        }
    }
}

double
ConjugateGradient::getResidual(const Vectord&)
{
    return m_cgSolver.error();
}

void
ConjugateGradient::setTolerance(const double epsilon)
{
    IterativeLinearSolver::setTolerance(epsilon);
    m_cgSolver.setTolerance(epsilon);
}

void
ConjugateGradient::setMaxNumIterations(const size_t maxIter)
{
    IterativeLinearSolver::setMaxNumIterations(maxIter);
    m_cgSolver.setMaxIterations(maxIter);
}

void
ConjugateGradient::setSystem(std::shared_ptr<LinearSystem<SparseMatrixd>> newSystem)
{
    LinearSolver<SparseMatrixd>::setSystem(newSystem);
    m_cgSolver.compute(m_linearSystem->getMatrix());
}

void
ConjugateGradient::print() const
{
    IterativeLinearSolver::print();

    LOG(INFO) << "Solver: Conjugate gradient";
    LOG(INFO) << "Tolerance: " << m_tolerance;
    LOG(INFO) << "max. iterations: " << m_maxIterations;
}

void
ConjugateGradient::solve(Vectord& x, const double tolerance)
{
    this->setTolerance(tolerance);
    this->solve(x);
}
} // namespace imstk