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
#include "Solvers/DirectLinearSolver.h"

BackwardEuler::BackwardEuler(OdeSystem *odeSystem): TimeIntegrator(odeSystem)
{}

//---------------------------------------------------------------------------
void BackwardEuler::solve(const OdeSystemState &state,
                          OdeSystemState &newState,
                          double timeStep)
{
    if(!this->system)
    {
        return;
    }

    // This function updates the state.
    auto updateIterate = [&](const core::Vectord &dv, core::Vectord &v)
    {
        v += dv;
        newState.getPositions() = state.getPositions() + timeStep*v;
    };

    // Function to evaluate the nonlinear objective function.
    auto G = [&,this](const core::Vectord &) -> const core::Vectord&
    {
        this->system->computeImplicitSystemRHS(state,newState,timeStep);
        return this->system->getRHS();
    };

    // Jacobian of the objective function.
    auto DG = [&,this](const core::Vectord &) -> const core::SparseMatrixd&
    {
        this->system->computeImplicitSystemLHS(state,newState,timeStep,false);
        return this->system->getSystemMatrix();
    };

    this->newtonSolver.setUpdateIterate(updateIterate);
    this->newtonSolver.setSystem(G);
    this->newtonSolver.setJacobian(DG);
    this->newtonSolver.setRelativeTolerance(0);
    this->newtonSolver.setLinearSolver(
            std::make_shared<DirectLinearSolver<core::SparseMatrixd>>());
    this->newtonSolver.solve(newState.getVelocities());
}
