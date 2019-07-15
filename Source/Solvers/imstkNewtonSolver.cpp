/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#include <iostream>

#include "imstkNewtonSolver.h"
#include "imstkIterativeLinearSolver.h"
#include "imstkConjugateGradient.h"

namespace imstk
{
NewtonSolver::NewtonSolver() :
    m_linearSolver(std::make_shared<ConjugateGradient>()),
    m_forcingTerm(0.9),
    m_absoluteTolerance(1e-3),
    m_relativeTolerance(1e-6),
    m_gamma(0.9),
    m_etaMax(0.9),
    m_maxIterations(1),
    m_useArmijo(true)
{
}

void
NewtonSolver::solveGivenState(Vectord& x)
{
    if (!m_nonLinearSystem)
    {
        LOG(WARNING) << "NewtonMethod::solve - nonlinear system is not set to the nonlinear solver";
        return;
    }

    // Compute norms, set tolerances and other temporaries
    double fnorm         = m_nonLinearSystem->evaluateF(x, m_isSemiImplicit).norm();
    double stopTolerance = m_absoluteTolerance + m_relativeTolerance * fnorm;

    m_linearSolver->setTolerance(stopTolerance);

    Vectord dx = x;

    for (size_t i = 0; i < m_maxIterations; ++i)
    {
        if (fnorm < stopTolerance)
        {
            return;
        }
        this->updateJacobian(x);
        m_linearSolver->solve(dx);
        m_updateIterate(-dx, x);

        double newNorm = fnorm;

        newNorm = this->armijo(dx, x, fnorm);

        if (m_forcingTerm > 0.0 && newNorm > stopTolerance)
        {
            double ratio = newNorm / fnorm; // Ratio of successive residual norms
            this->updateForcingTerm(ratio, stopTolerance, fnorm);

            // Reset tolerance in the linear solver according to the new forcing term
            // to avoid over solving of the system.
            m_linearSolver->setTolerance(m_forcingTerm);
        }

        fnorm = newNorm;
    }
}

void
NewtonSolver::solve()
{
    if (!m_nonLinearSystem)
    {
        LOG(WARNING) << "NewtonMethod::solve - nonlinear system is not set to the nonlinear solver";
        return;
    }

    size_t  iterNum;
    auto&   u  = m_nonLinearSystem->getUnknownVector();
    Vectord du = u; // make this a class member in future

    double error0, error;
    double epsilon = m_relativeTolerance * m_relativeTolerance;
    for (iterNum = 0; iterNum < m_maxIterations; ++iterNum)
    {
        error = updateJacobian(u);

        if (iterNum == 0)
        {
            error0 = error;
        }

        if (error / error0 < epsilon && iterNum > 0)
        {
            //std::cout << "Num. of Newton Iterations: " << i << "\tError ratio: " << error/error0 << std::endl;
            break;
        }

        m_linearSolver->solve(du);
        m_nonLinearSystem->m_FUpdate(du, m_isSemiImplicit);
    }
    m_nonLinearSystem->m_FUpdatePrevState();

    if (iterNum == m_maxIterations && !m_isSemiImplicit)
    {
        LOG(WARNING) << "NewtonMethod::solve - The solver did not converge after max. iterations";
    }
}

double
NewtonSolver::updateJacobian(const Vectord& x)
{
    // Evaluate the Jacobian and sets the matrix
    if (!m_nonLinearSystem)
    {
        LOG(WARNING) << "NewtonMethod::updateJacobian - nonlinear system is not set to the nonlinear solver";
        return -1;
    }

    auto& A = m_nonLinearSystem->m_dF(x);
    if (A.innerSize() == 0)
    {
        LOG(WARNING) << "NewtonMethod::updateJacobian - Size of matrix is 0!";
        return -1;
    }

    auto& b = m_nonLinearSystem->m_F(x, m_isSemiImplicit);

    auto linearSystem = std::make_shared<LinearSolverType::LinearSystemType>(A, b);
    //linearSystem->setLinearProjectors(m_nonLinearSystem->getLinearProjectors()); // TODO: Left for near future reference. Clear in future.
    m_linearSolver->setSystem(linearSystem);

    return b.dot(b);
}

void
NewtonSolver::updateForcingTerm(const double ratio, const double stopTolerance, const double fnorm)
{
    double eta = m_gamma * ratio * ratio;
    double forcingTermSqr = m_forcingTerm * m_forcingTerm;

    // Save guard to prevent the forcing term to become too small for far away iterates
    if (m_gamma * forcingTermSqr > 0.1)
    {
        // TODO: Log this
        eta = std::max(eta, m_gamma * forcingTermSqr);
    }

    m_forcingTerm = std::max(std::min(eta, m_etaMax), 0.5 * stopTolerance / fnorm);
}

void
NewtonSolver::setLinearSolver(std::shared_ptr<NewtonSolver::LinearSolverType> newLinearSolver)
{
    m_linearSolver = newLinearSolver;
}

std::shared_ptr<NewtonSolver::LinearSolverType>
NewtonSolver::getLinearSolver() const
{
    return m_linearSolver;
}

void
NewtonSolver::setAbsoluteTolerance(const double aTolerance)
{
    m_absoluteTolerance = aTolerance;
}

double
NewtonSolver::getAbsoluteTolerance() const
{
    return m_absoluteTolerance;
}
} // imstk
