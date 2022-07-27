/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkLinearSolver.h"
#include "imstkMath.h"

#ifdef WIN32
#pragma warning( push )
#pragma warning( disable : 4127 )
#endif
#include <Eigen/Sparse>
#include <Eigen/SparseLU>
#ifdef WIN32
#pragma warning( pop )
#endif

namespace imstk
{
template<typename MatrixType> class DirectLinearSolver;

///
/// \class DirectLinearSolver
///
/// \brief Dense direct solvers. Solves a dense system of equations using Cholesky
///     decomposition.
///
template<>
class DirectLinearSolver<Matrixd>: public LinearSolver<Matrixd>
{
public:
    DirectLinearSolver() { }
    DirectLinearSolver(const Matrixd& A, const Vectord& b);
    ~DirectLinearSolver() override { }

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
    /// \todo: only works for SPB matrices
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
    DirectLinearSolver() = default;
    ~DirectLinearSolver() override = default;

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
    Eigen::SparseLU<SparseMatrixd, Eigen::COLAMDOrdering<MatrixType::StorageIndex>> m_solver;
};
} // namespace imstk