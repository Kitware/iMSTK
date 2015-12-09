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

///
/// @brief Approximates the solution to the Initial value problem: dx/dt = F(x,t),
///     v(t0) = v0 using a forward Euler scheme; v(t1) = v(t0) + dtF(v(t0),t0)
///
class ForwardEuler : public TimeIntegrator
{
public:
    ///
    /// @brief Constructor/Destructor.
    ///
    ForwardEuler() = default;
    ~ForwardEuler() = default;

    ///
    /// @brief Perform one iteration of the Forward Euler method.
    ///
    /// \param x Current iterate.
    /// \param timeStep current timeStep.
    ///
    void solve(core::Vectord &x, double timeStep) override;

};

#endif // FORWARDEULER_H
