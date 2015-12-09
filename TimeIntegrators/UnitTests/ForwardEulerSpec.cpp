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

#include <memory>

// SimMedTK includes
#include "../ForwardEuler.h"

#include <bandit/bandit.h>
using namespace bandit;

go_bandit([]()
{
    describe("Explicit Euler ODE Solver", []()
    {
        auto euler = std::make_shared<ForwardEuler>();
        // ODE parameters
        double dt = 0.1;
        double t0 = 0, t1 = 3;
        size_t steps = (t1-t0)/dt;
        double lambda = -10;
        double a = 1;

        // ODE right hand side function
        auto F = [&](const core::Vectord &x, core::Vectord &y) -> core::Vectord&
        {
            y = lambda*x;
            return y;
        };
        euler->setFunction(F);

        it("constructs ", [&]()
        {
            AssertThat(euler != nullptr, IsTrue());
        });

        it("solves dx/dt=lambda*x, x(0)=a ", [&]()
        {
            // Initial value
            core::Vectord x(1);
            x(0) = a;

            // Find the solution for t = [0,1)
            core::Vectord sol(steps), error(steps);
            sol(0) = x(0);
            error(0) = 0.0;
            for(size_t i = 1; i < steps; ++i)
            {
                euler->solve(x,dt);
                sol(i) = x(0);
                error(i) = sol(i)-std::exp(lambda*i*dt);
            }
            AssertThat(error.norm(), IsLessThan(dt*steps));
        });
    });
});
