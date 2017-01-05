// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
//
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "imstkConjugateGradient.h"

namespace imstk
{

ConjugateGradient::ConjugateGradient()
{
    m_cgSolver.setMaxIterations(m_maxIterations);
    m_cgSolver.setTolerance(m_tolerance);
}

ConjugateGradient::ConjugateGradient(const SparseMatrixd& A, const Vectord& rhs) : ConjugateGradient()
{
    this->setSystem(std::make_shared<LinearSystem<SparseMatrixd>>(A, rhs));
}

void
ConjugateGradient::filter(Vectord& x, const std::vector<size_t>& filter)
{
    size_t threeI;
    for (auto &i: filter)
    {
        threeI = 3 * i;
        x(threeI) = x(threeI + 1) = x(threeI + 2) = 0.0;
    }
}

void
ConjugateGradient::solve(Vectord& x)
{
    if(!m_linearSystem)
    {
        LOG(WARNING) << "ConjugateGradient::solve: Linear system is not supplied for CG solver!";
        return;
    }

    if (m_linearSystem->getFilter().size() == 0)
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
    const auto &fixedNodes = m_linearSystem->getFilter();
    const auto &b = m_linearSystem->getRHSVector();
    const auto &A = m_linearSystem->getMatrix();

    // Set the initial guess to zero
    x.setZero();

    auto res = b;
    filter(res, fixedNodes);
    auto c = res;
    auto delta = res.dot(res);
    auto deltaPrev = delta;
    const auto eps = m_tolerance*m_tolerance*delta;
    double alpha = 0.0;
    double dotval;
    auto q = Vectord(b.size()).setZero();
    size_t iterNum = 0;

    while (delta > eps)
    {
        q = A * c;
        filter(q, fixedNodes);
        dotval = c.dot(q);
        if (dotval != 0.0)
        {
            alpha = delta / dotval;
        }
        else
        {
            LOG(WARNING) << "ConjugateGradient::modifiedCGSolve: deniminator zero. Terminating MCG iteation!";
            return;
        }
        x += alpha * c;
        res -= alpha * q;
        deltaPrev = delta;
        delta = res.dot(res);
        c *= delta / deltaPrev;
        c += res;
        filter(c, fixedNodes);

        if (++iterNum >= m_maxIterations)
        {
            LOG(WARNING) << "ConjugateGradient::modifiedCGSolve - The solver did not converge after max. iterations";
            break;
        }
    }
}

double
ConjugateGradient::getResidual(const Vectord& )
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

} // imstk