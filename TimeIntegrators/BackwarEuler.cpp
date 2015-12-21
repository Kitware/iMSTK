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

#include "BackwarEuler.h"

void BackwardEuler::solve(core::Vectord &x0, double timeStep)
{
    if(!this->F || !this->DF)
    {
        return;
    }

    auto G = [ &, this](const core::Vectord & x, core::Vectord & y) -> core::Vectord &
    {
        this->F(x, y);
        y = x - (x0 + timeStep * y);
        return y;
    };

    core::SparseMatrixd I(x0.size(), x0.size());
    I.setIdentity();
    auto DG = [ &, this](const core::Vectord & x, core::SparseMatrixd & J) -> void
    {
        this->DF(x, J);
        J = I - timeStep * J;
    };

    auto NewtonSolver = std::make_shared<InexactNewton>();
    auto solution = x0;

    NewtonSolver->setSystem(G);
    NewtonSolver->setJacobian(DG);
    NewtonSolver->solve(solution);
    x0 = solution;
}

//---------------------------------------------------------------------------
void BackwardEuler::setJacobian(const NonLinearSolver::JacobianType &newJacobian)
{
    this->DF = newJacobian;
}
