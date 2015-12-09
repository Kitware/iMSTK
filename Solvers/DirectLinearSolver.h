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

#ifndef SM_DIRECT_LINEAR_SOLVER
#define SM_DIRECT_LINEAR_SOLVER

// SimMedTK includes
#include "Solvers/LinearSolver.h"
#include "Core/Matrix.h"
#include "Core/Vector.h"

template<typename MatrixType>
class DirectLinearSolver;

///
/// \brief Dense direct solvers
///
template<>
class DirectLinearSolver<core::Matrixd> : public LinearSolver<core::Matrixd>
{
public:
    ///
    /// \brief Default constructor/destructor
    ///
    DirectLinearSolver() = delete;
    ~DirectLinearSolver() = default;

    ///
    /// \brief Constructor
    ///
    DirectLinearSolver(const core::Matrixd &matrix, const core::Vectord &b);

    ///
    /// \brief Solve the system of equations
    ///
    void solve(core::Vectord &x) override;

    ///
    /// \brief Solve the system of equations
    ///
    void solve(const core::Vectord &rhs, core::Vectord &x);

private:
    Eigen::LDLT<core::Matrixd> solver;
};

///
/// \brief Sparse direct solvers
///
template<>
class DirectLinearSolver<core::SparseMatrixd> : public LinearSolver<core::SparseMatrixd>
{
public:
    ///
    /// \brief Default constructor/destructor
    ///
    DirectLinearSolver() = delete;
    ~DirectLinearSolver() = default;

    ///
    /// \brief Constructor
    ///
    DirectLinearSolver(const core::SparseMatrixd &matrix, const core::Vectord &b);

    ///
    /// \brief Solve the system of equations
    ///
    void solve(core::Vectord &x) override;

    ///
    /// \brief Solve the system of equations
    ///
    void solve(const core::Vectord &rhs, core::Vectord &x);

private:
    Eigen::SparseLU<core::SparseMatrixd> solver;
};
#endif // SM_DIRECT_LINEAR_SOLVER
