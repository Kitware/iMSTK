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

#include "Solvers/DirectLinearSolver.h"
#include "Solvers/SystemOfEquations.h"


DirectLinearSolver<core::Matrixd>::DirectLinearSolver(const core::Matrixd &matrix, const core::Vectord &b)
{
    this->linearSystem = std::make_shared<LinearSystem<core::Matrixd>>(matrix, b);
    this->solver.compute(matrix);
}

//---------------------------------------------------------------------------
void DirectLinearSolver<core::Matrixd>::solve(core::Vectord &x)
{
    x.setZero();

    if(!this->linearSystem)
    {
        return;
    }

    auto b = this->linearSystem->getRHSVector();
    x = this->solver.solve(b);
}

//---------------------------------------------------------------------------
void DirectLinearSolver<core::Matrixd>::solve(const core::Vectord &rhs, core::Vectord &x)
{
    x = this->solver.solve(rhs);
}

//---------------------------------------------------------------------------
DirectLinearSolver<core::SparseMatrixd>::DirectLinearSolver(const core::SparseMatrixd &matrix, const core::Vectord &b)
{
    this->linearSystem = std::make_shared<LinearSystem<core::SparseMatrixd>>(matrix, b);
    this->solver.compute(matrix);
}

//---------------------------------------------------------------------------
void DirectLinearSolver<core::SparseMatrixd>::solve(core::Vectord &x)
{
    x.setZero();

    if(!this->linearSystem)
    {
        return;
    }

    auto b = this->linearSystem->getRHSVector();
    x = this->solver.solve(b);
}

//---------------------------------------------------------------------------
void DirectLinearSolver<core::SparseMatrixd>::solve(const core::Vectord &rhs, core::Vectord &x)
{
    x = this->solver.solve(rhs);
}
