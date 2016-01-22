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

#ifndef SOLVERS_DIRECTLINEARSOLVER_H
#define SOLVERS_DIRECTLINEARSOLVER_H

// iMSTK includes
#include "Solvers/LinearSolver.h"
#include "Core/Matrix.h"
#include "Core/Vector.h"

template<typename MatrixType>
class DirectLinearSolver;

///
/// \brief Dense direct solvers. Solves a dense system of equations using Cholesky
///     decomposition.
///
template<>
class DirectLinearSolver<core::Matrixd> : public LinearSolver<core::Matrixd>
{
public:
    ///
    /// \brief Default constructor/destructor.
    ///
    DirectLinearSolver() = delete;
    ~DirectLinearSolver() = default;

    ///
    /// \brief Constructor
    ///
    /// \param A System matrix. Symmetric and positive definite.
    /// \param b Right hand side of the linear system of equations.
    ///
    DirectLinearSolver(const core::Matrixd &A, const core::Vectord &b);

    ///
    /// \brief Solve the system of equations.
    ///
    /// \param x Upon return it contains the solution of the linear system.
    ///
    void solve(core::Vectord &x) override;

    ///
    /// \brief Solve the system of equations for arbitrary right hand side vector.
    ///
    /// \param rhs Right hand side of the linear system of equations.
    /// \param x Upon return it contains the solution of the linear system.
    ///
    void solve(const core::Vectord &rhs, core::Vectord &x);

    ///
    /// \brief Sets the system. System of linear equations.
    ///
    /// \param newSystem Linear system replacement.
    ///
    void setSystem(std::shared_ptr<LinearSystemType> newSystem) override;

private:
    Eigen::LDLT<core::Matrixd> solver;
};

///
/// \brief Sparse direct solvers. Solves a sparse system of equations using a sparse LU
///     decomposition.
///
template<>
class DirectLinearSolver<core::SparseMatrixd> : public LinearSolver<core::SparseMatrixd>
{
public:
    ///
    /// \brief Default constructor/destructor
    ///
    DirectLinearSolver() = default;
    ~DirectLinearSolver() = default;

    ///
    /// \brief Constructor
    ///
    /// \param matrix System matrix. Symmetric and positive definite.
    /// \param b Right hand side of the linear system of equations.
    ///
    DirectLinearSolver(const core::SparseMatrixd &matrix, const core::Vectord &b);

    ///
    /// \brief Solve the system of equations
    ///
    /// \param x Upon return it contains the solution of the linear system.
    ///
    void solve(core::Vectord &x) override;

    ///
    /// \brief Solve the system of equations for arbitrary right hand side vector.
    ///
    /// \param rhs Right hand side of the linear system of equations.
    /// \param x Upon return it contains the solution of the linear system.
    ///
    void solve(const core::Vectord &rhs, core::Vectord &x);

    ///
    /// \brief Sets the system. System of linear equations.
    ///
    /// \param newSystem Linear system replacement.
    ///
    void setSystem(std::shared_ptr<LinearSystemType> newSystem) override;

private:
    Eigen::SparseLU<core::SparseMatrixd> solver;
};
#endif // SOLVERS_DIRECT_LINEAR_SOLVER
