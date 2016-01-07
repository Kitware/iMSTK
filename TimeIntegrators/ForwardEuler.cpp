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

#include "ForwardEuler.h"

ForwardEuler::ForwardEuler() :
    linearSolver(std::make_shared<ConjugateGradient>()) {}

//---------------------------------------------------------------------------
ForwardEuler::ForwardEuler(OdeSystem *odeSystem): TimeIntegrator(odeSystem),
    linearSolver(std::make_shared<ConjugateGradient>()) {}

//---------------------------------------------------------------------------
void ForwardEuler::solve(const OdeSystemState &state,
                         OdeSystemState &newState,
                         double timeStep)
{
    this->computeSystemMatrix(state,newState,timeStep);

    auto linearSystem = std::make_shared<LinearSolverType::LinearSystemType>(
                            this->systemMatrix, this->rhs);

    this->linearSolver->setSystem(linearSystem);

    newState.getPositions() = state.getPositions() + timeStep * state.getVelocities();

    this->solution = state.getVelocities();
    this->linearSolver->solve(this->solution);

    newState.getVelocities() += this->solution;
}

//---------------------------------------------------------------------------
void ForwardEuler::computeSystemMatrix(const OdeSystemState &state,
                                       OdeSystemState &,
                                       double timeStep, bool computeRHS)
{
    auto &M = this->system->evalMass(state);

    this->systemMatrix = (1.0 / timeStep) * M;
    state.applyBoundaryConditions(this->systemMatrix);

    if(computeRHS)
    {
        this->rhs = this->system->evalF(state);
        state.applyBoundaryConditions(this->rhs);
    }
}
