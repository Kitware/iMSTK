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

#include "imstkNonLinearSolver.h"

namespace imstk
{

NonLinearSolver::NonLinearSolver(): m_sigma(std::array < double, 2 > {{0.1, 0.5}}),
                                    m_alpha(1e-4),
                                    m_armijoMax(30)
{
    this->m_updateIterate = [](const Vectord& dx, Vectord& x)
    {
        x += dx;
    };
}

double
NonLinearSolver::armijo(const Vectord& dx, Vectord& x, const double previousFnorm)
{
    /// Temporaries used in the line search
    std::array<double, 3> fnormSqr  = {previousFnorm*previousFnorm, 0.0, 0.0};
    std::array<double, 3> lambda    = {this->m_sigma[0]*this->m_sigma[1], 1.0, 1.0};

    /// Initialize temporaries
    if(!this->m_nonLinearSystem)
    {
        // TODO: log this
        return previousFnorm;
    }

    double currentFnorm = this->m_nonLinearSystem->m_F(x).norm();

    // Exit if the function norm satisfies the Armijo-Goldstein condition
    if(currentFnorm < (1.0 - this->m_alpha * lambda[0])*previousFnorm)
    {
        // TODO: Log this
        return currentFnorm;
    }

    // Save iterate in case this fails
    auto x_old = x;

    // Starts Armijo line search loop
    size_t i;
    for(i = 0; i < this->m_armijoMax; ++i)
    {
        /// Update x and keep books on lambda
        this->m_updateIterate(-lambda[0]*dx,x);
        lambda[2] = lambda[1];
        lambda[1] = lambda[0];

        currentFnorm = this->m_nonLinearSystem->m_F(x).norm();

        // Exit if the function norm satisfies the Armijo-Goldstein condition
        if(currentFnorm < (1.0 - this->m_alpha * lambda[0])*previousFnorm)
        {
        // TODO: Log this
            return currentFnorm;
        }

        /// Update function norms
        fnormSqr[2] = fnormSqr[1];
        fnormSqr[1] = currentFnorm * currentFnorm;

        /// Apply the three point parabolic model
        this->parabolicModel(fnormSqr, lambda);
    }

    if(i == this->m_armijoMax)
    {
        // TODO: Add to logger
//         std::cout << "Maximum number of Armijo iterations reached." << std::endl;
    }


    return currentFnorm;
}

void
NonLinearSolver::parabolicModel(const std::array<double,3> &fnorm, std::array<double,3> &lambda)
{
    /// Compute the coefficients for the interpolation polynomial:
    ///     p(lambda) = fnorm[0] + (b*lambda + a*lambda^2)/d1, where
    ///         d1 = (lambda[1] - lambda[2])*lambda[1]*lambda[2] < 0
    ///     if a > 0, then we have a concave up curvature and lambda defaults to:
    ///         lambda = sigma[0]*lambda
    double a1 = lambda[2] * (fnorm[1] - fnorm[0]);
    double a2 = lambda[1] * (fnorm[2] - fnorm[0]);
    double a = a1 - a2;

    if(a >= 0)
    {
        lambda[0] = this->m_sigma[0] * lambda[1];
        return;
    }

    double b = lambda[1] * a2 - lambda[2] * a1;
    double newLambda = -.5 * b / a;

    if(newLambda < this->m_sigma[0] * lambda[1])
    {
        newLambda = this->m_sigma[0] * lambda[1];
    }

    if(newLambda > this->m_sigma[1] * lambda[1])
    {
        newLambda = this->m_sigma[1] * lambda[1];
    }

    lambda[0] = newLambda;
}

void
NonLinearSolver::setSigma(const std::array<double,2> &newSigma)
{
    this->m_sigma = newSigma;
}

const std::array<double,2> &
NonLinearSolver::getSigma() const
{
    return this->m_sigma;
}

void
NonLinearSolver::setAlpha(const double newAlpha)
{
    this->m_alpha = newAlpha;
}

double
NonLinearSolver::getAlpha() const
{
    return this->m_alpha;
}

void
NonLinearSolver::setArmijoMax(const size_t newArmijoMax)
{
    this->m_armijoMax = newArmijoMax;
}

size_t
NonLinearSolver::getArmijoMax() const
{
    return this->m_armijoMax;
}

void
NonLinearSolver::setSystem(std::shared_ptr<NonLinearSystem> newSystem)
{
    this->m_nonLinearSystem = newSystem;
}

std::shared_ptr<NonLinearSystem>
NonLinearSolver::getSystem() const
{
    return this->m_nonLinearSystem;
}

void
NonLinearSolver::setUpdateIterate(const NonLinearSolver::UpdateIterateType& newUpdateIterate)
{
    this->m_updateIterate = newUpdateIterate;
}

} // imstk