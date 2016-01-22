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
#ifndef SM_SPARSE_CG
#define SM_SPARSE_CG

#include <memory>

// SimMedTK includes
#include "Solvers/IterativeLinearSolver.h"
#include "Solvers/SystemOfEquations.h"

// Eigen includes
#include <Eigen/IterativeLinearSolvers>

///
/// \brief Conjugate gradient sparse linear solver for SPD matrices
///
class ConjugateGradient : public IterativeLinearSolver
{
public:
    ///
    /// \brief Constructors/Destructor
    ///
    ConjugateGradient() = default;
    ~ConjugateGradient() = default;

    ///
    /// \brief Constructor
    ///
    /// \param A System matrix. Symmetric and positive definite.
    /// \param rhs Right hand side of the linear system of equations.
    ///
    ConjugateGradient(const core::SparseMatrixd &A, const core::Vectord &rhs);
    ConjugateGradient(const ConjugateGradient &) = delete;
    ConjugateGradient &operator=(const ConjugateGradient &) = delete;

    ///
    /// \brief Do one iteration of the method.
    ///
    /// \param x Current iterate.
    /// \param updateResidual Compute residual if true.
    ///
    void iterate(core::Vectord &x, bool updateResidual = true) override;

    ///
    /// \brief Solve the system of equations.
    ///
    /// \param x Upon return it contains the approximate solution of the linear system.
    ///
    void solve(core::Vectord &x) override;

    ///
    /// \brief Solve the linear system using Conjugate gradient iterations to a
    ///     specified tolerance.
    ///
    /// \param tolerance Specified tolerance.
    ///
    void solve(core::Vectord &x, double tolerance);

    ///
    /// \brief Return the error calculated by the solver.
    ///
    double getError(const core::Vectord &x) override;

    ///
    /// \brief Set the tolerance for the iterative solver.
    ///
    /// \param tolerance Specified tolerance.
    ///
    void setTolerance(const double tolerance) override;

    ///
    /// \brief Set the maximum number of iterations for the iterative solver
    ///
    /// \param maxIter Specified maximum tolerance.
    ///
    void setMaximumIterations(const size_t maxIter) override;

    ///
    /// \brief Sets the system. System of linear equations.
    ///
    /// \param newSystem Linear system replacement.
    ///
    void setSystem(std::shared_ptr<LinearSystemType> newSystem) override;

    ///
    /// \brief Print solver information
    ///
    void print();

private:
    ///> Pointer to the Eigen's Conjugate gradient solver
    Eigen::ConjugateGradient<core::SparseMatrixd> solver;
};

#endif // SM_SPARSE_CG
