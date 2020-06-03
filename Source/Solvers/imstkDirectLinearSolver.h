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

#pragma warning( push )
#pragma warning( disable : 4127 )
#include <Eigen/Sparse>
#include <Eigen/SparseLU>
#pragma warning( pop )

// iMSTK includes
#include "imstkLinearSolver.h"
#include "imstkMath.h"

namespace imstk
{
template<typename MatrixType> class DirectLinearSolver;

///
/// \brief Dense direct solvers. Solves a dense system of equations using Cholesky
///     decomposition.
///
template<>
class DirectLinearSolver<Matrixd>: public LinearSolver<Matrixd>
{
public:
    ///
    /// \brief Default constructor/destructor.
    ///
    // DirectLinearSolver() = delete;
    DirectLinearSolver() {}
    ~DirectLinearSolver() {};

    ///
    /// \brief Constructor
    ///
    DirectLinearSolver(const Matrixd& A, const Vectord& b);

    ///
    /// \brief Solve the system of equations.
    ///
    void solve(Vectord& x) override;

    ///
    /// \brief Solve the system of equations for arbitrary right hand side vector.
    ///
    void solve(const Vectord& rhs, Vectord& x);

    ///
    /// \brief Sets the system. System of linear equations.
    ///
    void setSystem(std::shared_ptr<LinearSystemType> newSystem) override;

    ///
    /// \brief Sets the matrix
    ///
    void setSystem(std::shared_ptr<Matrixd> matrix);

    ///
    /// \brief Returns true if the solver is iterative
    ///
    bool isIterative() const override
    {
        return false;
    };

private:
    Eigen::LDLT<Matrixd> m_solver;
};

///
/// \brief Sparse direct solvers. Solves a sparse system of equations using a sparse LU
///     decomposition.
///
template<>
class DirectLinearSolver<SparseMatrixd>: public LinearSolver<SparseMatrixd>
{
public:
    ///
    /// \brief Default constructor/destructor
    ///
    DirectLinearSolver()  = default;
    ~DirectLinearSolver() = default;

    ///
    /// \brief Constructor
    ///
    DirectLinearSolver(const SparseMatrixd& matrix, const Vectord& b);

    ///
    /// \brief Sets the system. System of linear equations.
    ///
    void setSystem(std::shared_ptr<LinearSystemType> newSystem) override;

    ///
    /// \brief Solve the system of equations
    ///
    void solve(Vectord& x) override;

    ///
    /// \brief Solve the system of equations for arbitrary right hand side vector.
    ///
    void solve(const Vectord& rhs, Vectord& x);

private:
    Eigen::SparseLU<SparseMatrixd, Eigen::COLAMDOrdering<MatrixType::StorageIndex>> m_solver;//?
};
} // imstk
