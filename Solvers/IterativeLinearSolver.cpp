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

#include "Solvers/SparseLinearSolver.h"

IterativeLinearSolver::IterativeLinearSolver(
    const int epsilon,
    const int maxIter)
    : tolerance(epsilon), maxIterations(maxIter), LinearSolver()
{
}

IterativeLinearSolver::IterativeLinearSolver(
    const std::shared_ptr<SparseLinearSystem> linSys/*=nullptr*/,
    const int epsilon /*= 1.0e-6*/,
    const int maxIter /*= 100*/)
    : tolerance(epsilon), maxIterations(maxIter), LinearSolver()
{
}

IterativeLinearSolver::~IterativeLinearSolver()
{
}


void IterativeLinearSolver::setTolerance(const double epsilon)
{
    this->tolerance = epsilon;
}

void IterativeLinearSolver::setMaximumIterations(const int maxIter)
{
    this->maxIterations = maxIter;
}

int IterativeLinearSolver::getTolerance() const
{
    return this->tolerance;
}

int IterativeLinearSolver::getMaximumIterations() const
{
    return this->maxIterations;
}
