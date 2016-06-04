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

ConjugateGradient::ConjugateGradient(const SparseMatrixd& A, const Vectord& rhs) : cgSolver(A)
{
    m_linearSystem = std::make_shared<LinearSystem<SparseMatrixd>>(A, rhs);
    m_maxIterations = rhs.size();
    m_tolerance = 1.0e-6;

    cgSolver.setMaxIterations(m_maxIterations);
    cgSolver.setTolerance(m_tolerance);
    cgSolver.compute(A);
}

void
ConjugateGradient::iterate(Vectord& , bool)
{
    // Nothing to do
}

void
ConjugateGradient::solve(Vectord& x)
{
    if(!this->m_linearSystem)
    {
        // TODO: Log this
        return;
    }
    x = cgSolver.solve(m_linearSystem->getRHSVector());
}

double
ConjugateGradient::getResidual(const Vectord& )
{
    return cgSolver.error();
}

void
ConjugateGradient::setTolerance(const double epsilon)
{
    IterativeLinearSolver::setTolerance(epsilon);
    cgSolver.setTolerance(epsilon);
}

void
ConjugateGradient::setMaxNumIterations(const size_t maxIter)
{
    IterativeLinearSolver::setMaxNumIterations(maxIter);
    cgSolver.setMaxIterations(maxIter);
}

void
ConjugateGradient::setSystem(std::shared_ptr<LinearSystem<SparseMatrixd>> newSystem)
{
    LinearSolver<SparseMatrixd>::setSystem(newSystem);
    this->cgSolver.compute(this->m_linearSystem->getMatrix());
}

void
ConjugateGradient::print()
{
    IterativeLinearSolver::print();

    LOG(INFO) << "Solver: Conjugate gradient";
    LOG(INFO) << "Tolerance: " << m_tolerance;
    LOG(INFO) << "max. iterations: " << m_maxIterations;
}

void
ConjugateGradient::solve(Vectord& x, const double tolerance)
{
    setTolerance(tolerance);
    solve(x);
}

}
