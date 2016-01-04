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

#ifndef BACKWAREULER_H
#define BACKWAREULER_H

#include "TimeIntegrators/TimeIntegrator.h"

#include "Core/Vector.h"
#include "Solvers/InexactNewton.h"
#include "TimeIntegrators/OdeSystem.h"

///
/// @brief Approximates the solution to the IVP: Mdv/dt = f(x,v,t), x(t0) = x0 with
///     a backward Euler scheme.
///
class BackwardEuler : public TimeIntegrator
{
public:
    using SystemMatrixType = OdeSystem::MatrixFunctionType;

public:
    ///
    /// @brief Default constructor/destructor.
    ///
    BackwardEuler() = default;
    ~BackwardEuler() = default;

    ///
    /// \brief Constructor. Takes the system describing the ODE.
    ///
    BackwardEuler(std::shared_ptr<OdeSystem> system) : TimeIntegrator(system)
    {}

    ///
    /// @brief Perform one iteration of the Backward Euler method.
    ///
    /// \param state Current iterate.
    /// \param newState New computed state.
    /// \param timeStep Time step used to discretize  the system.
    ///
    void solve(const OdeSystemState &state, OdeSystemState &newState, double timeStep) override;

    ///
    /// \brief Compute and store the system matrix.
    ///
    /// \param state Current state
    /// \param newState New state
    /// \param timeStep Time step used to discretize the ODE.
    ///
    void computeSystemMatrix(const OdeSystemState &state,
                             OdeSystemState &newState,
                             const double timeStep,
                             bool computeRHS = true)
    {
        auto &M = this->system->evalMass(newState);
        auto &K = this->system->evalDFv(newState);
        auto &C = this->system->evalDFx(newState);

        this->systemMatrix = (1.0/timeStep) * M;
        this->systemMatrix += C;
        this->systemMatrix += timeStep * K;
        state.applyBoundaryConditions(this->systemMatrix);

        if(computeRHS)
        {
            this->rhs = this->system->evalF(newState) + K*(newState.getPositions() -
                        state.getPositions() - newState.getVelocities()*timeStep);
            this->rhs -= M*(newState.getVelocities()-state.getVelocities())/timeStep;
            state.applyBoundaryConditions(this->rhs);
        }
    }

    ///
    /// \brief Compute and store the right hand side of the system.
    ///
    /// \param state Current state
    /// \param newState New state
    /// \param timeStep Time step used to discretize the ODE.
    ///
    void computeSystemRHS(const OdeSystemState &state,
                          OdeSystemState &newState,
                          double timeStep)
    {
        auto &M = this->system->evalMass(newState);
        auto &K = this->system->evalDFv(newState);

        this->rhs = this->system->evalF(newState) + K*(newState.getPositions() -
                    state.getPositions() - newState.getVelocities()*timeStep);
        this->rhs -= M*(newState.getVelocities()-state.getVelocities())/timeStep;
        state.applyBoundaryConditions(this->rhs);
    }
};

#endif // BACKWAREULER_H
