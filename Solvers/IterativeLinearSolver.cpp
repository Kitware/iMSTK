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

#include "Solvers/IterativeLinearSolver.h"

namespace imstk {

IterativeLinearSolver::IterativeLinearSolver()
    : maxIterations(100)
{
}

//---------------------------------------------------------------------------
void IterativeLinearSolver::solve(Vectord &x)
{
    if(!this->linearSystem)
    {
        // TODO: Log this
        return;
    }
    auto epsilon  = this->minTolerance * this->minTolerance;
    this->linearSystem->computeResidual(x, this->residual);

    for(size_t i = 0; i < this->maxIterations; ++i)
    {
        if(this->residual.squaredNorm() < epsilon)
        {
            return;
        }

        this->iterate(x);
    }
}

//---------------------------------------------------------------------------
void IterativeLinearSolver::setMaximumIterations(const size_t maxIter)
{
    this->maxIterations = maxIter;
}

//---------------------------------------------------------------------------
size_t IterativeLinearSolver::getMaximumIterations() const
{
    return this->maxIterations;
}

//---------------------------------------------------------------------------
const Vectord &IterativeLinearSolver::getResidual()
{
    return this->residual;
}

//---------------------------------------------------------------------------
double IterativeLinearSolver::getError(const Vectord &x)
{
    this->linearSystem->computeResidual(x, this->residual);
    return this->residual.squaredNorm();
}

//---------------------------------------------------------------------------
void IterativeLinearSolver::print()
{
    // Nothing to print
}

}
