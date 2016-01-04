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

#include "TimeIntegrators/OdeSystem.h"

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
void OdeSystem::setFunction(OdeSystem::FunctionType newF)
{
    this->F = newF;
}

//---------------------------------------------------------------------------
const core::SparseMatrixd &OdeSystem::evalDFx(const OdeSystemState &s)
{
    return this->DFx(s);
}

//---------------------------------------------------------------------------
const core::SparseMatrixd &OdeSystem::evalDFv(const OdeSystemState &s)
{
    return this->DFv(s);
}

//---------------------------------------------------------------------------
const core::SparseMatrixd &OdeSystem::evalMass(const OdeSystemState &s)
{
    return this->Mass(s);
}

//---------------------------------------------------------------------------
const core::Vectord &OdeSystem::evalF(const OdeSystemState &s)
{
    return this->F(s);
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
