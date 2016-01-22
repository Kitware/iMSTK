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

#include "TimeIntegrators/OdeSystem.h"
#include <iostream>

void OdeSystem::setJaconbianFv(OdeSystem::MatrixFunctionType newDFv)
{
    this->DFv = newDFv;
}

//---------------------------------------------------------------------------
void OdeSystem::setJaconbianFx(OdeSystem::MatrixFunctionType newDFx)
{
    this->DFx = newDFx;
}

//---------------------------------------------------------------------------
void OdeSystem::setMass(OdeSystem::MatrixFunctionType newMass)
{
    this->Mass = newMass;
}

//---------------------------------------------------------------------------
void OdeSystem::setDamping(OdeSystem::MatrixFunctionType newDamping)
{
    this->Damping = newDamping;
}

//---------------------------------------------------------------------------
void OdeSystem::setFunction(OdeSystem::FunctionType newF)
{
    this->F = newF;
}

//---------------------------------------------------------------------------
const std::shared_ptr< OdeSystemState > OdeSystem::getInitialState() const
{
    return this->initialState;
}

//---------------------------------------------------------------------------
void OdeSystem::setInitialState(std::shared_ptr< OdeSystemState > newState)
{
    this->initialState = newState;
}

//---------------------------------------------------------------------------
void OdeSystem::computeImplicitSystemLHS(const OdeSystemState &previousState,
                                    OdeSystemState &newState,
                                    const double timeStep,
                                    bool computeRHS)
{
    if(!this->Mass || !this->DFx || !this->DFv || !this->F)
    {
        // TODO: Log this, fatal error
        return;
    }
    auto &M = this->evalMass(newState);
    auto &K = this->evalDFx(newState);
    auto &C = this->evalDFv(newState);

    this->systemMatrix = (1.0 / timeStep) * M;
    this->systemMatrix += C;
    this->systemMatrix += timeStep * K;

    if(this->Damping)
    {
        this->systemMatrix += this->Damping(newState);
    }

    previousState.applyBoundaryConditions(this->systemMatrix);

    if(computeRHS)
    {
        auto &f = this->evalF(newState);
        this->rhs = M * (newState.getVelocities() -
                         previousState.getVelocities()) / timeStep;
        this->rhs -= (f + K * (newState.getPositions() - previousState.getPositions() -
                             newState.getVelocities() * timeStep));

        if(this->Damping)
        {
            this->rhs -= timeStep*this->Damping(newState)*newState.getVelocities();
        }
        previousState.applyBoundaryConditions(this->rhs);
    }
}

//---------------------------------------------------------------------------
void OdeSystem::computeExplicitSystemLHS(const OdeSystemState &state,
                                            OdeSystemState &,
                                            double timeStep, bool computeRHS)
{
    auto &M = this->evalMass(state);

    this->systemMatrix = (1.0 / timeStep) * M;
    state.applyBoundaryConditions(this->systemMatrix);

    if(computeRHS)
    {
        this->rhs = this->evalF(state);
        state.applyBoundaryConditions(this->rhs);
    }
}

//---------------------------------------------------------------------------
void OdeSystem::computeImplicitSystemRHS(const OdeSystemState &state,
                                 OdeSystemState &newState,
                                 double timeStep)
{
    if(!this->Mass || !this->DFx || !this->DFv || !this->F)
    {
        // TODO: Log this, fatal error
        return;
    }
    auto &M = this->evalMass(newState);
    auto &K = this->evalDFx(newState);
    auto &f = this->evalF(newState);

    this->rhs = M * (newState.getVelocities() - state.getVelocities()) / timeStep;
    this->rhs -= K * (newState.getPositions() - state.getPositions() -
                           newState.getVelocities() * timeStep);
    this->rhs += f;

    if(this->Damping)
    {
        this->rhs -= timeStep*this->Damping(newState)*newState.getVelocities();
    }
    state.applyBoundaryConditions(this->rhs);
}

//---------------------------------------------------------------------------
const core::SparseMatrixd &OdeSystem::getSystemMatrix() const
{
    return this->systemMatrix;
}

//---------------------------------------------------------------------------
core::SparseMatrixd &OdeSystem::getSystemMatrix()
{
    return this->systemMatrix;
}

//---------------------------------------------------------------------------
void OdeSystem::setSystemMatrix(const core::SparseMatrixd &newMatrix)
{
    this->systemMatrix = newMatrix;
}

//---------------------------------------------------------------------------
const core::Vectord &OdeSystem::getRHS() const
{
    return this->rhs;
}

//---------------------------------------------------------------------------
core::Vectord &OdeSystem::getRHS()
{
    return this->rhs;
}

//---------------------------------------------------------------------------
void OdeSystem::setRHS(const core::Vectord &newRhs)
{
    this->rhs = newRhs;
}
