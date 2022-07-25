/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkLinearSolver.h"

namespace imstk
{
///
/// \class IterativeLinearSolver
///
/// \brief Base class for iterative linear solvers.
///
class IterativeLinearSolver : public LinearSolver<SparseMatrixd>
{
public:
    IterativeLinearSolver() = default;
    ~IterativeLinearSolver() override = default;

    ///
    /// \brief Do one iteration of the method.
    ///
    //virtual void iterate(Vectord& x, bool updateResidual = true) = 0;

    ///
    /// \brief set/get the maximum number of iterations for the iterative solver.
    ///
    virtual void setMaxNumIterations(const size_t maxIter);
    virtual size_t getMaxNumIterations() const;

    ///
    /// \brief Return residual vector
    ///
    virtual const Vectord& getResidualVector();
    virtual const Vectord& getResidualVector(const Vectord& x);

    ///
    /// \brief Return residue in 2-norm
    ///
    virtual double getResidual(const Vectord& x);

    ///
    /// \brief Print solver information.
    ///
    void print() const override;

    ///
    /// \brief Solve the linear system using Gauss-Seidel iterations.
    ///
    virtual void solve(Vectord& x) override;

    ///
    /// \brief Returns true if the solver is iterative
    ///
    bool isIterative() const override
    {
        return true;
    }

protected:
    size_t  m_maxIterations = 100;  ///< Maximum number of iterations to be performed.
    Vectord m_residual;             ///< Storage for residual vector.
};
} // namespace imstk