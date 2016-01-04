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

#ifndef FORWARDEULER_H
#define FORWARDEULER_H

#include "TimeIntegrators/TimeIntegrator.h"
#include "Core/Vector.h"
#include "Solvers/ConjugateGradient.h"

///
/// @brief Approximates the solution to the Initial value problem: Mdv/dt = F(x,v,t),
///     v(t0) = v0 using a forward Euler scheme; Mv(t1) = Mv(t0) + dtF(v(t0),t0)
///
class ForwardEuler : public TimeIntegrator
{
public:
    using LinearSolverType = LinearSolver<core::SparseMatrixd>;

public:
    ///
    /// @brief Default constructor/destructor.
    ///
    ForwardEuler():
        linearSolver(std::make_shared<ConjugateGradient>()){}
    ~ForwardEuler() = default;

    ///
    /// @brief Perform one iteration of the Forward Euler method.
    ///
    /// \param x Current iterate.
    /// \param timeStep Current timeStep.
    ///
    void solve(const OdeSystemState &state,OdeSystemState &newState,double timeStep) override;

    ///
    /// \brief Compute and store the system matrix.
    ///
    /// \param state Current state
    /// \param newState New state
    /// \param timeStep Time step used to discretize the ODE.
    ///
    void computeSystemMatrix(const OdeSystemState &state, OdeSystemState &, double timeStep, bool computeRHS = true)
    {
        auto &M = this->system->evalMass(state);

        this->systemMatrix = (1.0/timeStep) * M;
        state.applyBoundaryConditions(this->systemMatrix);

        if(computeRHS)
        {
            this->rhs = this->system->evalF(state);
            state.applyBoundaryConditions(this->rhs);
        }
    }

private:
    std::shared_ptr<LinearSolverType> linearSolver; ///> Linear solver to use. Default: Conjugate gradient.
    core::Vectord solution; ///> Solution to the linear solve.
};

#endif // FORWARDEULER_H
