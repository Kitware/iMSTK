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

#include <memory>

// iMSTK includes
#include "../BackwarEuler.h"

#include <bandit/bandit.h>
using namespace bandit;

go_bandit([]()
{
    describe("Implicit Euler ODE Solver", []()
    {
        auto odeSystem = std::make_shared<OdeSystem>();
        auto euler = std::make_shared<BackwardEuler>();
        auto initialState = std::make_shared<OdeSystemState>();
        // ODE parameters
        double dt = 0.01;
        double t0 = 0, t1 = 3;
        size_t steps = (t1-t0)/dt;
        double lambda = -10;
        double a = 1;

        // ODE right hand side function
        core::Vectord y;
        auto F = [&](const OdeSystemState &x) -> const core::Vectord&
        {
            y = -lambda*x.getVelocities();
            return y;
        };

        std::vector<Eigen::Triplet<double>> tripletList;
        tripletList.emplace_back(0,0,0);
        core::SparseMatrixd K(1,1);
        K.setFromTriplets(tripletList.begin(),tripletList.end());
        K.makeCompressed();
        auto DFx = [&](const OdeSystemState &) -> const core::SparseMatrixd&
        {
            return K;
        };

        core::SparseMatrixd C(1,1);
        tripletList.clear();
        tripletList.emplace_back(0,0,-lambda);
        C.setFromTriplets(tripletList.begin(),tripletList.end());
        C.makeCompressed();
        auto DFv = [&](const OdeSystemState &) -> const core::SparseMatrixd&
        {
            return C;
        };

        core::SparseMatrixd M(1,1);
        tripletList.clear();
        tripletList.emplace_back(0,0,1.0);
        M.setFromTriplets(tripletList.begin(),tripletList.end());
        M.makeCompressed();
        auto Mass = [&](const OdeSystemState &) -> const core::SparseMatrixd&
        {
            return M;
        };

        odeSystem->setRHSFunction(F);
        odeSystem->setJaconbianFx(DFx);
        odeSystem->setJaconbianFv(DFv);
        odeSystem->setMass(Mass);

        it("constructs ", [&]()
        {
            AssertThat(euler != nullptr, IsTrue());
            AssertThat(odeSystem != nullptr, IsTrue());
        });

        // Initial state
        initialState->resize(1);
        initialState->getVelocities()(0) = a;

        odeSystem->setInitialState(initialState);
        euler->setSystem(odeSystem.get());

        it("solves dx/dt-lambda*x=0, x(0)=a ", [&]()
        {
            // Find the solution for t = [0,1)
            OdeSystemState state, newState;
            state = *initialState;
            newState = state;

            core::Vectord error(steps), sol(steps);
            error(0) = 0.0;
            sol(0) = a;
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
