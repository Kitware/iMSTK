/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkIterativeLinearSolver.h"

#include <Eigen/IterativeLinearSolvers>

namespace imstk
{
class LinearProjectionConstraint;

///
/// \class ConjugateGradient
///
/// \brief Conjugate gradient sparse linear solver for Spd matrices
///
class ConjugateGradient : public IterativeLinearSolver
{
public:
    ConjugateGradient();
    ConjugateGradient(const SparseMatrixd& A, const Vectord& rhs);
    ~ConjugateGradient() override = default;

    ///
    /// \brief Dissallow copy & move
    ///
    ConjugateGradient(const ConjugateGradient&)  = delete;
    ConjugateGradient(const ConjugateGradient&&) = delete;
    ConjugateGradient& operator=(const ConjugateGradient&)  = delete;
    ConjugateGradient& operator=(const ConjugateGradient&&) = delete;

    ///
    /// \brief Do one iteration of the method.
    ///
    //void iterate(Vectord& x, bool updateResidual = true) override {};

    ///
    /// \brief Solve the system of equations.
    ///
    void solve(Vectord& x) override;

    ///
    /// \brief Solve the linear system using Conjugate gradient iterations to a
    ///     specified tolerance.
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
    virtual void setMaxNumIterations(const size_t maxIter) override;

    ///
    /// \brief Set solver tolerance
    ///
    void setTolerance(const double tolerance);

    ///
    /// \brief Print solver information
    ///
    void print() const override;

    ///
    /// \brief Apply a filter to the vector supplied
    ///
    void applyLinearProjectionFilter(Vectord& x, const std::vector<LinearProjectionConstraint>& linProj, const bool setVal);

    ///
    /// \brief Get the vector denoting the filter
    ///
    void setLinearProjectors(std::vector<LinearProjectionConstraint>* f)
    {
        m_FixedLinearProjConstraints = f;
    }

    /// \brief Get the vector denoting the filter
    ///
    std::vector<LinearProjectionConstraint>& getLinearProjectors()
    {
        return *m_FixedLinearProjConstraints;
    }

    /// \brief Get the vector denoting the filter
    ///
    void setDynamicLinearProjectors(std::vector<LinearProjectionConstraint>* f)
    {
        m_DynamicLinearProjConstraints = f;
    }

    /// \brief Get the vector denoting the filter
    ///
    std::vector<LinearProjectionConstraint>& getDynamicLinearProjectors()
    {
        return *m_DynamicLinearProjConstraints;
    }

private:
    ///
    /// \brief Modified Conjugate gradient solver
    ///
    void modifiedCGSolve(Vectord& x);

    ///< Pointer to the Eigen's Conjugate gradient solver
    Eigen::ConjugateGradient<SparseMatrixd> m_cgSolver;

    std::vector<LinearProjectionConstraint>* m_FixedLinearProjConstraints   = nullptr;
    std::vector<LinearProjectionConstraint>* m_DynamicLinearProjConstraints = nullptr;
};
} // namespace imstk