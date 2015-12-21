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

#ifndef TIMEINTEGRATOR_H
#define TIMEINTEGRATOR_H

#include "Core/Vector.h"
#include "Solvers/SystemOfEquations.h" // For FunctionType

///
/// @brief Base class for all time time integrators
///
class TimeIntegrator
{
public:
    using FunctionType = SystemOfEquations::FunctionType;

public:
    ///
    /// @brief Constructor/Destructor
    ///
    TimeIntegrator() = default;
    ~TimeIntegrator() = default;

    ///
    /// @brief Perform one iteration of the method
    ///
    /// \param x Current iterate.
    /// \param timeStep current timeStep
    ///
    virtual void solve(core::Vectord&,double) = 0;

    ///
    /// \brief Set the the right hand side function of the ODE system
    ///
    void setFunction(const FunctionType &newF);

protected:
    FunctionType F;
};

#endif // TIMEINTEGRATOR_H
