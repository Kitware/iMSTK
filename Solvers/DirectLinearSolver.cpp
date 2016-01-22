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

#include "Solvers/DirectLinearSolver.h"
#include "Solvers/SystemOfEquations.h"
#include <iostream>

DirectLinearSolver<core::Matrixd>::
DirectLinearSolver(const core::Matrixd &matrix, const core::Vectord &b)
{
    this->linearSystem = std::make_shared<LinearSystem<core::Matrixd>>(matrix, b);
    this->solver.compute(matrix);
}

//---------------------------------------------------------------------------
void DirectLinearSolver<core::Matrixd>::solve(core::Vectord &x)
{
    if(!this->linearSystem)
    {
        return;
    }

    x.setZero();

    auto b = this->linearSystem->getRHSVector();
    x = this->solver.solve(b);
}

//---------------------------------------------------------------------------
void DirectLinearSolver<core::Matrixd>::solve(const core::Vectord &rhs, core::Vectord &x)
{
    x = this->solver.solve(rhs);
}

//---------------------------------------------------------------------------
void DirectLinearSolver<core::Matrixd>::
setSystem(std::shared_ptr<LinearSystem<core::Matrixd>> newSystem)
{
    LinearSolver<core::Matrixd>::setSystem(newSystem);
    this->solver.compute(this->linearSystem->getMatrix());
}

//---------------------------------------------------------------------------
DirectLinearSolver<core::SparseMatrixd>::
DirectLinearSolver(const core::SparseMatrixd &matrix, const core::Vectord &b)
{
    this->linearSystem = std::make_shared<LinearSystem<core::SparseMatrixd>>(matrix, b);
    this->solver.compute(matrix);
}

//---------------------------------------------------------------------------
void DirectLinearSolver<core::SparseMatrixd>::solve(core::Vectord &x)
{
    if(!this->linearSystem)
    {
        return;
    }

    x.setZero();

    auto b = this->linearSystem->getRHSVector();
    x = this->solver.solve(b);
}

//---------------------------------------------------------------------------
void DirectLinearSolver<core::SparseMatrixd>::
solve(const core::Vectord &rhs, core::Vectord &x)
{
    x = this->solver.solve(rhs);
}

//---------------------------------------------------------------------------
void DirectLinearSolver<core::SparseMatrixd>::
setSystem(std::shared_ptr<LinearSystem<core::SparseMatrixd>> newSystem)
{
    LinearSolver<core::SparseMatrixd>::setSystem(newSystem);
    this->solver.compute(this->linearSystem->getMatrix());
}
