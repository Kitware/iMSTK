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
#include <iostream>

ConjugateGradient::ConjugateGradient(const core::SparseMatrixd &A,
                                     const core::Vectord &rhs) : solver(A)
{
    this->linearSystem = std::make_shared<LinearSystem<core::SparseMatrixd>>(A, rhs);
    this->solver.setMaxIterations(this->maxIterations);
    this->solver.setTolerance(this->minTolerance);
}

//---------------------------------------------------------------------------
void ConjugateGradient::iterate(core::Vectord &, bool)
{
    std::cout << "Nothing to do\n";
}

//---------------------------------------------------------------------------
void ConjugateGradient::solve(core::Vectord &x)
{
    if(this->linearSystem)
    {
        x = this->solver.solve(this->linearSystem->getRHSVector());
    }

    this->linearSystem->computeResidual(x, this->residual);
}

//---------------------------------------------------------------------------
void ConjugateGradient::solve(core::Vectord &x, double tolerance)
{
    this->setTolerance(tolerance);
    this->solver.setTolerance(tolerance);
    this->solve(x);
}

//---------------------------------------------------------------------------
double ConjugateGradient::getError() const
{
    return this->solver.error();
}
