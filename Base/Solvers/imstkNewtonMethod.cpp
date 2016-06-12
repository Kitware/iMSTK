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

#include <iostream>

#include "imstkNewtonMethod.h"
#include "imstkIterativeLinearSolver.h"
#include "imstkConjugateGradient.h"


namespace imstk
{

NewtonMethod::NewtonMethod():
    linearSolver(std::make_shared<ConjugateGradient>()),
    forcingTerm(0.9),
    absoluteTolerance(1e-3),
    relativeTolerance(1e-6),
    gamma(0.9),
    etaMax(0.9),
    maxIterations(50),
    useArmijo(true) {}


void
NewtonMethod::solve(Vectord& x)
{
    if(!this->m_nonLinearSystem)
    {
        // TODO: log this
        return;
    }

    // Compute norms, set tolerances and other temporaries
    double fnorm = this->m_nonLinearSystem->evaluateF(x).norm();
    double stopTolerance = this->absoluteTolerance + this->relativeTolerance * fnorm;

    this->linearSolver->setTolerance(stopTolerance);

    Vectord dx = x;

    for(size_t i = 0; i < this->maxIterations; ++i)
    {
        if(fnorm < stopTolerance)
        {
            /// TODO Log this
            return;
        }
        this->updateJacobian(x);
        this->linearSolver->solve(dx);
        this->m_updateIterate(-dx,x);

        double newNorm = fnorm;

        newNorm = this->armijo(dx, x, fnorm);

        if(this->forcingTerm > 0.0 && newNorm > stopTolerance)
        {
            double ratio = newNorm / fnorm; // Ratio of successive residual norms
            this->updateForcingTerm(ratio, stopTolerance, fnorm);

            // Reset tolerance in the linear solver according to the new forcing term
            // to avoid over solving of the system.
            this->linearSolver->setTolerance(this->forcingTerm);
        }

        fnorm = newNorm;
    }
}


void
NewtonMethod::updateForcingTerm(const double ratio, const double stopTolerance, const double fnorm)
{
    double eta = this->gamma * ratio * ratio;
    double forcingTermSqr = this->forcingTerm * this->forcingTerm;

    // Save guard to prevent the forcing term to become too small for far away iterates
    if(this->gamma * forcingTermSqr > .1)
    {
        // TODO: Log this
        eta = std::max(eta, this->gamma * forcingTermSqr);
    }

    this->forcingTerm = std::max(std::min(eta, this->etaMax), 0.5 * stopTolerance / fnorm);
}


void
NewtonMethod::setLinearSolver(std::shared_ptr< NewtonMethod::LinearSolverType > newLinearSolver)
{
    this->linearSolver = newLinearSolver;
}


std::shared_ptr<NewtonMethod::LinearSolverType>
NewtonMethod::getLinearSolver() const
{
    return this->linearSolver;
}


void
NewtonMethod::updateJacobian(const Vectord& x)
{
    // Evaluate the Jacobian and sets the matrix
    if(!this->m_nonLinearSystem)
    {// TODO: Print message or log error.
        return;
    }

    auto &A = this->m_nonLinearSystem->m_dF(x);

    if(A.innerSize() == 0)
    {// TODO: Print message and/or log error.
        return;
    }

    auto &b = this->m_nonLinearSystem->m_F(x);

    auto linearSystem = std::make_shared<LinearSolverType::LinearSystemType>(A, b);
    this->linearSolver->setSystem(linearSystem);
}


void
NewtonMethod::setAbsoluteTolerance(const double aTolerance)
{
    this->absoluteTolerance = aTolerance;
}


double
NewtonMethod::getAbsoluteTolerance() const
{
    return this->absoluteTolerance;
}

}
