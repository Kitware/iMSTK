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
        auto odeSystem = std::make_shared<OdeSystem>();
        auto initialState = std::make_shared<OdeSystemState>();
        // ODE parameters
        double dt = 0.1;
        double t0 = 0, t1 = 3;
        size_t steps = (t1-t0)/dt;
        double lambda = -10;
        double a = 1;

        // ODE right hand side function
        core::Vectord y;
        auto F = [&](const OdeSystemState &x) -> core::Vectord&
        {
            y = lambda*x.getVelocities();
            return y;
        };
        odeSystem->setFunction(F);

        // ODE system matrix
        std::vector<Eigen::Triplet<double>> tripletList;
        tripletList.emplace_back(0,0,1);
        core::SparseMatrixd M(1,1);
        M.setFromTriplets(tripletList.begin(),tripletList.end());
        M.makeCompressed();
        auto Mass = [&](const OdeSystemState &) -> const core::SparseMatrixd
        {
            return M;
        };
        odeSystem->setMass(Mass);

        it("constructs ", [&]()
        {
            AssertThat(euler != nullptr, IsTrue());
            AssertThat(odeSystem != nullptr, IsTrue());
        });

        // Initial state
        initialState->resize(1);
        initialState->getVelocities()[0] = a;

        odeSystem->setInitialState(initialState);
        euler->setSystem(odeSystem);

        it("solves dx/dt=lambda*x, x(0)=a ", [&]()
        {
            // Initial value
            core::Vectord x(1);
            x(0) = a;

            // Find the solution for t = [0,1)
            OdeSystemState state, newState;
            state = *initialState;

            core::Vectord sol(steps), error(steps);
            sol(0) = a;
            error(0) = 0.0;
            for(size_t i = 1; i < steps; ++i)
            {
                euler->solve(state,newState,dt);
                state = newState;
                sol(i) = state.getVelocities()(0);
                error(i) = sol(i)-std::exp(lambda*i*dt);
            }
            AssertThat(error.norm(), IsLessThan(dt*steps));
        });
    });
});
