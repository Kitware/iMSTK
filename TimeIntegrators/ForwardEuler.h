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

#ifndef TIME_INTEGRATORS_FORWARDEULER_H
#define TIME_INTEGRATORS_FORWARDEULER_H

#include "TimeIntegrators/TimeIntegrator.h"
#include "Core/Vector.h"
#include "Solvers/ConjugateGradient.h"

namespace imstk {

///
/// @brief Approximates the solution to the Initial value problem: Mdv/dt = F(x,v,t),
///     v(t0) = v0 using a forward Euler scheme; Mv(t1) = Mv(t0) + dtF(v(t0),t0)
///
class ForwardEuler : public TimeIntegrator
{
public:
    using LinearSolverType = LinearSolver<SparseMatrixd>;

public:
    ///
    /// @brief Default Constructor/Destructor.
    ///
    ForwardEuler();
    ~ForwardEuler() = default;

    ///
    /// \brief Constructor. Takes the system describing the ODE.
    ///
    ForwardEuler(OdeSystem *odeSystem);

    ///
    /// @brief Perform one iteration of the Forward Euler method.
    ///
    /// \param x Current iterate.
    /// \param timeStep Current timeStep.
    ///
    void solve(const OdeSystemState &state,
               OdeSystemState &newState,
               double timeStep) override;

private:
    std::shared_ptr<LinearSolverType> linearSolver; ///> Linear solver to use. (Default: ConjugateGradient)
};

}

#endif // FORWARDEULER_H
