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

#pragma once

// STL includes
#include <array>
#include <functional>

// imstk includes
#include "imstkNonLinearSystem.h"
#include "imstkMath.h"
#include "imstkSolverBase.h"

namespace imstk
{
///
/// \brief Base class for non-linear solvers
///
template <typename SystemMatrix>
class NonLinearSolver : public SolverBase
{
public:
    using JacobianType      = std::function<const SystemMatrix& (const Vectord&)>;
    using UpdateIterateType = std::function<void (const Vectord&, Vectord&)>;
    using FunctionType      = NonLinearSystem<SparseMatrixd>::VectorFunctionType;

public:
    ///
    /// \brief Default Constructor/Destructor
    ///
    NonLinearSolver();
    virtual ~NonLinearSolver() = default;

    ///
    /// \brief Main solve routine.
    ///
    virtual void solveGivenState(Vectord& x) = 0;
    virtual void solve() override = 0;

    ///
    /// \brief Backtracking line search method based on the Armijo-Goldstein condition
    ///
    /// \param dx Computed direction.
    /// \param x Current iterate.
    ///
    double armijo(const Vectord& dx, Vectord& x, const double previousFnorm);

    ///
    /// \brief Three-point safeguarded parabolic model for a line search. Upon return
    ///     lambda[0] will contain the new value of lambda given by the parabolic model.
    ///
    /// \param lambda[1] Current step length
    ///        lambda[2] Previous step length
    /// \param fnorm[0] Value of |F(x)|^2
    ///        fnorm[1] Value of |F(x + lambda[1]*dx)|^2
    ///        fnorm[2] Value of |F(x + lambda[2]*dx)|^2
    ///
    void parabolicModel(const std::array<double, 3>& fnorm,
                        std::array<double, 3>& lambda);

    ///
    /// \brief Set/Get Sigma. Safeguard parameter for the the line search method.
    ///
    /// \param newSigma New sigma parameter.
    ///
    void setSigma(const std::array<double, 2>& newSigma);
    const std::array<double, 2>& getSigma() const;

    ///
    /// \brief Set/Get Alpha. Parameter to measure sufficient decrease in the line search.
    ///
    /// \param newAlpha New alpha parameter.
    ///
    void setAlpha(const double newAlpha);
    double getAlpha() const;

    ///
    /// \brief Set/Get ArmijoMax. Maximum number of step length reductions.
    ///
    /// \param newArmijoMax New iteration parameter.
    ///
    void setArmijoMax(const size_t newArmijoMax);
    size_t getArmijoMax() const;

    ///
    /// \brief Sets the system. System of nonlinear equations.
    ///
    /// \param newSystem Non-linear system replacement.
    ///
    void setSystem(std::shared_ptr<NonLinearSystem<SystemMatrix>> newSystem);
    std::shared_ptr<NonLinearSystem<SystemMatrix>> getSystem() const;

    ///
    /// \brief Set a customized iterate update function.
    ///
    /// \param newUpdateIterate Function used to update iterates. Default: x+=dx.
    ///
    void setUpdateIterate(const UpdateIterateType& newUpdateIterate)
	{
		m_updateIterate = newUpdateIterate;
	}

    ///
    /// \brief Set the Newton solver to be fully implicit
    ///
    virtual void setToFullyImplicit()
    {
        m_isSemiImplicit = false;
    }

    ///
    /// \brief Set the Newton solver to be fully implicit
    ///
    virtual void setToSemiImplicit()
    {
        m_isSemiImplicit = true;
    }

protected:
    std::array<double, 2> m_sigma;                      ///< Safeguarding bounds for the line search
    double m_alpha;                                     ///< Parameter to measure decrease
    size_t m_armijoMax;                                 ///< Maximum number of step length reductions

    std::shared_ptr<NonLinearSystem<SystemMatrix>> m_nonLinearSystem; ///< System of non-linear equations
    UpdateIterateType m_updateIterate;                  ///< Update iteration function
    bool m_isSemiImplicit = true;                       ///> Semi-Implicit solver
};
} // imstk
