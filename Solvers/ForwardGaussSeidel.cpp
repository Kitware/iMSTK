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

#include "Solvers/ForwardGaussSeidel.h"

ForwardGaussSeidel::ForwardGaussSeidel(
    const core::SparseMatrixd &A,
    const core::Vectord &rhs)
{
    this->linearSystem = std::make_shared<LinearSystem<core::SparseMatrixd>>(A, rhs);
}

//---------------------------------------------------------------------------
void ForwardGaussSeidel::iterate(core::Vectord &x, bool updateResidual)
{
    x = this->linearSystem->getRHSVector() -
        this->linearSystem->getStrictUpperTriangular() * x;

    this->linearSystem->getLowerTriangular().solveInPlace(x);

    if (updateResidual)
    {
        this->linearSystem->computeResidual(x, this->residual);
    }
}

//---------------------------------------------------------------------------
void ForwardGaussSeidel::relax(core::Vectord& x)
{
    const auto &A = this->linearSystem->getMatrix();
    const auto &b = this->linearSystem->getRHSVector();
    //
    // TODO: invD can be precomputed for further optimization.
    const auto invD = 1.0/A.diagonal().array();
    for (int k = 0; k < A.outerSize(); ++k)
    {
        double sum = b(k);
        for (core::SparseMatrixd::InnerIterator it(A,k); it; ++it)
        {
            sum -= it.value()*x(it.col());
        }
        x(k) += sum*invD(k);
    }
}

//---------------------------------------------------------------------------
void ForwardGaussSeidel::
setSystem(std::shared_ptr<LinearSystem<core::SparseMatrixd>> newSystem)
{
    LinearSolver::setSystem(newSystem);
}
