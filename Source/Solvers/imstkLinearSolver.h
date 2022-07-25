/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkLinearSystem.h"
#include "imstkMath.h"

namespace imstk
{
///
/// \class LinearSolver
///
/// \brief Base class for linear solvers
///
template<typename SystemMatrixType>
class LinearSolver
{
public:
    using MatrixType       = SystemMatrixType;
    using LinearSystemType = LinearSystem<MatrixType>;

    enum class Type
    {
        ConjugateGradient,
        LUFactorization,
        GaussSeidel,
        SuccessiveOverRelaxation,
        Jacobi,
        GMRES,
        None
    };

    ///
    /// \brief Default constructor/destructor
    ///
    LinearSolver() : m_linearSystem(nullptr) { }
    virtual ~LinearSolver() = default;

    ///
    /// \brief Main solve routine
    ///
    virtual void solve(Vectord& x) = 0;

    ///
    /// \brief Set/get the system. Replaces/Returns the stored linear system of equations.
    ///
    virtual void setSystem(std::shared_ptr<LinearSystemType> newSystem)
    {
        m_linearSystem.reset();
        m_linearSystem = newSystem;
    }

    std::shared_ptr<LinearSystemType> getSystem() const
    {
        return m_linearSystem;
    }

    ///
    /// \brief Set solver tolerance
    ///
    void setTolerance(const double tolerance)
    {
        m_tolerance = tolerance;
    }

    ///
    /// \brief Get solver tolerance
    ///
    double getTolerance() const
    {
        return m_tolerance;
    }

    ///
    /// \brief Print solver information.
    ///
    virtual void print() const
    {
        //LOG(INFO) << "Solver type (broad): Linear";
    }

    ///
    /// \brief Returns true if the solver is iterative
    ///
    virtual bool isIterative() const = 0;

    ///
    /// \brief Return the type of the solver
    ///
    Type getType() { return m_type; }

protected:
    Type   m_type      = Type::None;                  ///< Type of the scene object
    double m_tolerance = 1.0e-4;                      ///< default tolerance
    std::shared_ptr<LinearSystemType> m_linearSystem; ///< Linear system of equations
};
} // namespace imstk