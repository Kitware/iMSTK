// This file is part of the SimMedTK project.
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
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#include "Solvers/ConjugateGradient.h"

ConjugateGradient::
ConjugateGradient(const core::SparseMatrixd &A, const core::Vectord &rhs) : solver(A)
{
    this->linearSystem = std::make_shared<LinearSystem<core::SparseMatrixd>>(A, rhs);
    this->maxIterations = rhs.size();
    this->minTolerance = 1e-6;
    this->solver.setMaxIterations(this->maxIterations);
    this->solver.setTolerance(this->minTolerance);
    this->solver.compute(A);
}

//---------------------------------------------------------------------------
void ConjugateGradient::iterate(core::Vectord &, bool)
{
    // Nothing to do
}

//---------------------------------------------------------------------------
void ConjugateGradient::solve(core::Vectord &x)
{
    if(this->linearSystem)
    {
        x = this->solver.solve(this->linearSystem->getRHSVector());
    }
}

//---------------------------------------------------------------------------
void ConjugateGradient::solve(core::Vectord &x, double tolerance)
{
    this->setTolerance(tolerance);
    this->solve(x);
}

//---------------------------------------------------------------------------
double ConjugateGradient::getError(const core::Vectord &)
{
    return this->solver.error();
}

//---------------------------------------------------------------------------
void ConjugateGradient::setTolerance(const double epsilon)
{
    this->minTolerance = epsilon;
    this->solver.setTolerance(epsilon);
}

//---------------------------------------------------------------------------
void ConjugateGradient::setMaximumIterations(const size_t maxIter)
{
    this->maxIterations = maxIter;
    this->solver.setMaxIterations(maxIter);
}

//---------------------------------------------------------------------------
void ConjugateGradient::
setSystem(std::shared_ptr<LinearSystem<core::SparseMatrixd>> newSystem)
{
    LinearSolver<core::SparseMatrixd>::setSystem(newSystem);
    this->solver.compute(this->linearSystem->getMatrix());
}

//---------------------------------------------------------------------------
void ConjugateGradient::print()
{
    /// TODO: Log this
//     std::cout << "#iterations:     " << this->solver.iterations() << std::endl;
//     std::cout << "estimated error: " << this->solver.error()      << std::endl;
//     std::cout << "tolerance: " << this->solver.tolerance()      << std::endl;
//     std::cout << "maxIterations: " << this->solver.maxIterations() << std::endl;
}
