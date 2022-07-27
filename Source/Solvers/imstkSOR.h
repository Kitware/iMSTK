/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkNonLinearSystem.h"
#include "imstkIterativeLinearSolver.h"

#include <Eigen/IterativeLinearSolvers>

namespace imstk
{
class LinearProjectionConstraint;

///
/// \class SOR
///
/// \brief Successive Over Relaxation (SOR) sparse linear solver
///
class SOR : public IterativeLinearSolver
{
public:
    SOR(const double relaxationFactor = 0.5) : m_relaxationFactor(relaxationFactor) { m_type = Type::SuccessiveOverRelaxation; };
    SOR(const SparseMatrixd& A, const Vectord& rhs);
    ~SOR() override = default;

    ///
    /// \brief Dissallow copy & move
    ///
    SOR(const SOR&)  = delete;
    SOR(const SOR&&) = delete;
    SOR& operator=(const SOR&)  = delete;
    SOR& operator=(const SOR&&) = delete;

    ///
    /// \brief Do one iteration of the method.
    ///
    //void iterate(Vectord& x, bool updateResidual = true) override {};

    ///
    /// \brief Gauss-Seidel solver
    ///
    void SORSolve(Vectord& x);

    ///
    /// \brief Solve the system of equations.
    ///
    void solve(Vectord& x) override;

    ///
    /// \brief Solve the linear system using Gauss-Seidel iterations to a
    /// specified tolerance.
    ///
    void solve(Vectord& x, const double tolerance);

    ///
    /// \brief Return the error calculated by the solver.
    ///
    double getResidual(const Vectord& x) override;

    ///
    /// \brief Sets the system. System of linear equations.
    ///
    void setSystem(std::shared_ptr<LinearSystemType> newSystem) override;

    ///
    /// \brief set/get the maximum number of iterations for the iterative solver.
    ///
    void setMaxNumIterations(const size_t maxIter) override;

    ///
    /// \brief Set solver tolerance
    ///
    void setTolerance(const double tolerance);

    ///
    /// \brief Print solver information
    ///
    void print() const override;

    ///
    /// \brief Return the relaxation factor
    ///
    double getRelaxationFactor() const { return m_relaxationFactor; };

    ///
    /// \brief Get the vector denoting the filter
    ///
    void setLinearProjectors(std::vector<LinearProjectionConstraint>* f)
    {
        m_FixedLinearProjConstraints = f;
    }

    ///
    /// \brief Get the vector denoting the filter
    ///
    std::vector<LinearProjectionConstraint>& getLinearProjectors()
    {
        return *m_FixedLinearProjConstraints;
    }

    ///
    /// \brief Get the vector denoting the filter
    ///
    void setDynamicLinearProjectors(std::vector<LinearProjectionConstraint>* f)
    {
        m_DynamicLinearProjConstraints = f;
    }

    ///
    /// \brief Get the vector denoting the filter
    ///
    std::vector<LinearProjectionConstraint>& getDynamicLinearProjectors()
    {
        return *m_DynamicLinearProjConstraints;
    }

private:
    double m_relaxationFactor = 0.5;

    std::vector<LinearProjectionConstraint>* m_FixedLinearProjConstraints   = nullptr;
    std::vector<LinearProjectionConstraint>* m_DynamicLinearProjConstraints = nullptr;
};
} // namespace imstk