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

#include "BackwarEuler.h"
#include "Solvers/DirectLinearSolver.h"

namespace imstk {

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
    auto updateIterate = [&](const Vectord &dv, Vectord &v)
    {
        v += dv;
        newState.getPositions() = state.getPositions() + timeStep*v;
    };

    // Function to evaluate the nonlinear objective function.
    auto G = [&,this](const Vectord &) -> const Vectord&
    {
        this->system->computeImplicitSystemRHS(state,newState,timeStep);
        return this->system->getRHSVector();
    };

    // Jacobian of the objective function.
    auto DG = [&,this](const Vectord &) -> const SparseMatrixd&
    {
        this->system->computeImplicitSystemLHS(state,newState,timeStep,false);
        return this->system->getMatrix();
    };

    this->system->setFunction(G);
    this->system->setJacobian(DG);
    this->newtonSolver.setSystem(this->system);
    this->newtonSolver.setUpdateIterate(updateIterate);
    this->newtonSolver.setRelativeTolerance(.001/this->system->getRHSVector().norm());
    // For small systems use a direct solver
    if(state.getPositions().size() < 100)
    {
        this->newtonSolver.setLinearSolver(
            std::make_shared<DirectLinearSolver<SparseMatrixd>>());
    }
    this->newtonSolver.solve(newState.getVelocities());
}

}
