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

#ifndef NON_LINEAR_SOLVER
#define NON_LINEAR_SOLVER

// STL includes
#include<array>

// Simmedtk includes
#include "Solvers/SystemOfEquations.h"
#include "Core/Matrix.h"
#include "Core/Vector.h"

///
/// \brief Base class for non-linear solvers
///
class NonLinearSolver
{
public:
    using JacobianType = std::function<void(const core::Vectord &, core::SparseMatrixd &)>;
    using FunctionType = SystemOfEquations::FunctionType;

public:
    ///
    /// \brief Default Constructor/Destructor
    ///
    NonLinearSolver();
    virtual ~NonLinearSolver() = default;

    ///
    /// \brief Main solve routine.
    ///
    virtual void solve(core::Vectord &x) = 0;

    ///
    /// \brief Backtracking line search method based on the Armijo-Goldstein condition
    ///
    /// \param dx Computed direction.
    /// \param x Current iterate.
    ///
    double armijo(const core::Vectord &dx, core::Vectord &x);

    ///
    /// \brief Three-point safeguarded parabolic model for a line search. Upon return
    ///     lambda[0] will contain the new value of lambda given by the parabolic model.
    ///
    /// \param lambda[1] Current steplength
    ///        lambda[2] Previous steplength
    /// \param fnorm[0] Value of |F(x)|^2
    ///        fnorm[1] Value of |F(x + lambda[1]*dx)|^2
    ///        fnorm[2] Value of |F(x + lambda[2]*dx)|^2
    ///
    void parabolicModel(const std::array<double, 3> &fnorm,
                        std::array<double, 3> &lambda);

    ///
    /// \brief Set/Get Sigma. Safeguard parameter for the the linesearch method.
    ///
    /// \param newSigma New sigma parameter.
    ///
    void setSigma(const std::array<double, 2> &newSigma);
    const std::array<double, 2> &getSigma() const;

    ///
    /// \brief Set/Get Alpha. Parameter to measure sufficient decrease in the linerseach.
    ///
    /// \param newAlpha New alpha parameter.
    ///
    void setAlpha(const double newAlpha);
    double getAlpha() const;

    ///
    /// \brief Set/Get ArmijoMax. Maximum number of steplength reductions.
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
    void setSystem(std::shared_ptr<SystemOfEquations> newSystem);
    std::shared_ptr<SystemOfEquations> getSystem() const;

    ///
    /// \brief Set the system. This utility function allows you to set the system with
    ///     just a lambda function.
    ///
    /// \param F Function describing the nonlinear system.
    ///
    void setSystem(const FunctionType &F);

protected:
    core::Vectord f;                ///< Storage for function evaluations
    std::array<double, 2> sigma;    ///< Safeguarding bounds for the lineseach
    double alpha;                   ///< Parameter to measure decrease
    size_t armijoMax;               ///< Maximum number of steplength reductions
    std::shared_ptr<SystemOfEquations> nonLinearSystem; ///< System of non-linear equations
};

#endif // SM_LINEAR_SOLVER
