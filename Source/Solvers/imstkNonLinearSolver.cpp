/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkNonLinearSolver.h"

namespace imstk
{
template<typename SystemMatrix>
NonLinearSolver<SystemMatrix>::NonLinearSolver() : m_sigma(std::array<double, 2>
    {
        { 0.1, 0.5 }
    }),
    m_alpha(1e-4),
    m_armijoMax(30)
{
    m_updateIterate = [](const Vectord& dx, Vectord& x)
                      {
                          x += dx;
                      };
}

template<typename SystemMatrix>
double
NonLinearSolver<SystemMatrix>::armijo(const Vectord& dx, Vectord& x, const double previousFnorm)
{
    /// Temporaries used in the line search
    std::array<double, 3> fnormSqr = { previousFnorm* previousFnorm, 0.0, 0.0 };
    std::array<double, 3> lambda   = { m_sigma[0] * m_sigma[1], 1.0, 1.0 };

    /// Initialize temporaries
    if (!m_nonLinearSystem)
    {
        /// \todo log this
        return previousFnorm;
    }

    double currentFnorm = m_nonLinearSystem->m_F(x, !m_isSemiImplicit).norm();

    // Exit if the function norm satisfies the Armijo-Goldstein condition
    if (currentFnorm < (1.0 - m_alpha * lambda[0]) * previousFnorm)
    {
        /// \todo Log this
        return currentFnorm;
    }

    // Save iterate in case this fails
    //auto x_old = x;

    // Starts Armijo line search loop
    size_t i;
    for (i = 0; i < m_armijoMax; ++i)
    {
        /// Update x and keep books on lambda
        m_updateIterate(-lambda[0] * dx, x);
        lambda[2] = lambda[1];
        lambda[1] = lambda[0];

        currentFnorm = m_nonLinearSystem->m_F(x, !m_isSemiImplicit).norm();

        // Exit if the function norm satisfies the Armijo-Goldstein condition
        if (currentFnorm < (1.0 - m_alpha * lambda[0]) * previousFnorm)
        {
            /// \todo Log this
            return currentFnorm;
        }

        /// Update function norms
        fnormSqr[2] = fnormSqr[1];
        fnormSqr[1] = currentFnorm * currentFnorm;

        /// Apply the three point parabolic model
        this->parabolicModel(fnormSqr, lambda);
    }

    if (i == m_armijoMax)
    {
        /// \todo Add to logger
//         std::cout << "Maximum number of Armijo iterations reached." << std::endl;
    }
    return currentFnorm;
}

template<typename SystemMatrix>
void
NonLinearSolver<SystemMatrix>::parabolicModel(const std::array<double, 3>& fnorm, std::array<double, 3>& lambda)
{
    /// Compute the coefficients for the interpolation polynomial:
    ///     p(lambda) = fnorm[0] + (b*lambda + a*lambda^2)/d1, where
    ///         d1 = (lambda[1] - lambda[2])*lambda[1]*lambda[2] < 0
    ///     if a > 0, then we have a concave up curvature and lambda defaults to:
    ///         lambda = sigma[0]*lambda
    double a1 = lambda[2] * (fnorm[1] - fnorm[0]);
    double a2 = lambda[1] * (fnorm[2] - fnorm[0]);
    double a  = a1 - a2;

    if (a >= 0)
    {
        lambda[0] = m_sigma[0] * lambda[1];
        return;
    }

    double b = lambda[1] * a2 - lambda[2] * a1;
    double newLambda = -.5 * b / a;

    if (newLambda < m_sigma[0] * lambda[1])
    {
        newLambda = m_sigma[0] * lambda[1];
    }

    if (newLambda > m_sigma[1] * lambda[1])
    {
        newLambda = m_sigma[1] * lambda[1];
    }

    lambda[0] = newLambda;
}

template<typename SystemMatrix>
void
NonLinearSolver<SystemMatrix>::setSigma(const std::array<double, 2>& newSigma)
{
    m_sigma = newSigma;
}

template<typename SystemMatrix>
const std::array<double, 2>&
NonLinearSolver<SystemMatrix>::getSigma() const
{
    return m_sigma;
}

template<typename SystemMatrix>
void
NonLinearSolver<SystemMatrix>::setAlpha(const double newAlpha)
{
    m_alpha = newAlpha;
}

template<typename SystemMatrix>
double
NonLinearSolver<SystemMatrix>::getAlpha() const
{
    return m_alpha;
}

template<typename SystemMatrix>
void
NonLinearSolver<SystemMatrix>::setArmijoMax(const size_t newArmijoMax)
{
    m_armijoMax = newArmijoMax;
}

template<typename SystemMatrix>
size_t
NonLinearSolver<SystemMatrix>::getArmijoMax() const
{
    return m_armijoMax;
}

template<typename SystemMatrix>
void
NonLinearSolver<SystemMatrix>::setSystem(std::shared_ptr<NonLinearSystem<SystemMatrix>> newSystem)
{
    m_nonLinearSystem = newSystem;
}

template<typename SystemMatrix>
std::shared_ptr<NonLinearSystem<SystemMatrix>>
NonLinearSolver<SystemMatrix>::getSystem() const
{
    return m_nonLinearSystem;
}

template class NonLinearSolver<SparseMatrixd>;
template class NonLinearSolver<Matrixd>;
} // namespace imstk