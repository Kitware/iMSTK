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
#include "TimeIntegrators/OdeSystem.h"

///
/// @brief Base class for all time time integrators
///
class TimeIntegrator
{
public:
    using FunctionType = OdeSystem::FunctionType;

    enum IntegratorType
    {
        ImplicitEuler,
        ExplicitEuler
    };

public:
    ///
    /// @brief Constructor/Destructor
    ///
    TimeIntegrator() = default;
    virtual ~TimeIntegrator() = default;

    ///
    /// \brief Constructor. Takes the system describing the ODE.
    ///
    TimeIntegrator(OdeSystem *odeSystem);

    ///
    /// \brief Perform one iteration of the method
    ///
    /// \param oldState Previous state.
    /// \param newState New writable state.
    /// \param timeStep Step size.
    ///
    virtual void solve(const OdeSystemState &oldState,
                       OdeSystemState &newState,
                       const double timeStep) = 0;

    ///
    /// \brief Return the Ode system of equations.
    ///
    /// \return Pointer to ODE system.
    ///
    OdeSystem *getSystem() const;

    ///
    /// \brief Set a new system of ODEs.
    ///
    /// \param newSystem Ode system.
    ///
    void setSystem(OdeSystem *odeSystem);

protected:
    OdeSystem *system;  ///> System of differential equations.
};

#endif // TIMEINTEGRATOR_H
