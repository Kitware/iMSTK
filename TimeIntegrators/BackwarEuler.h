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

///
/// @brief Approximates the solution to the IVP: dv/dt = F(x,dx/dt,t), x(t0) = x0 with
///     a backward Euler scheme.
///
class BackwardEuler : public TimeIntegrator
{
public:
    ///
    /// @brief Constructor/Destructor.
    ///
    BackwardEuler() = default;
    ~BackwardEuler() = default;

    ///
    /// @brief Perform one iteration of the Backward Euler method.
    ///
    /// \param x Current iterate.
    /// \param timeStep current timeStep.
    ///
    void solve(core::Vectord &x0, double timeStep) override;

    ///
    /// @brief Set the Jacobian of the right hand side jacobian.
    ///
    /// \param newJacobian Function for updating the jacobian matrix.
    ///
    void setJacobian(const InexactNewton::JacobianType &newJacobian);

private:
    InexactNewton::JacobianType DF;
};

#endif // BACKWAREULER_H
