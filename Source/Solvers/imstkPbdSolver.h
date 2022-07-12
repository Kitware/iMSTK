/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkPbdConstraint.h"
#include "imstkSolverBase.h"

namespace imstk
{
class PbdConstraintContainer;

///
/// \class PbdSolver
///
/// \brief Position Based Dynamics solver
/// This solver can solve both partitioned constraints (unordered_set of vector'd constraints) in parallel
/// and sequentially on vector'd constraints. It requires a set of constraints, positions, and invMasses.
///
class PbdSolver : public SolverBase
{
public:
    PbdSolver();
    ~PbdSolver() override = default;

    ///
    /// \brief Set Iterations. The number of nonlinear iterations.
    ///
    void setIterations(const size_t iterations) { this->m_iterations = iterations; }

    ///
    /// \brief Sets the constraints the solver should solve for
    /// These wil be solved sequentially
    ///
    void setConstraints(std::shared_ptr<PbdConstraintContainer> constraints) { this->m_constraints = constraints; }

    ///
    /// \brief Add a constraint list to this solver to be solved, for quick addition/removal
    /// particularly collision.
    ///
    void addConstraints(std::vector<PbdConstraint*>* constraints)
    {
        m_constraintLists->push_back(constraints);
    }

    ///
    /// \brief Sets the bodies to solve with
    ///
    void setPbdBodies(PbdState* state) { this->m_state = state; }

    ///
    /// \brief Set time step
    ///
    void setTimeStep(const double dt) { m_dt = dt; }

    ///
    /// \brief Get Iterations. Returns current nonlinear iterations.
    ///
    size_t getIterations() const { return this->m_iterations; }

    ///
    /// \brief Set the PBD solver type
    ///
    void setSolverType(const PbdConstraint::SolverType& type) { m_solverType = type; }

    ///
    /// \brief Solve the non linear system of equations G(x)=0 using Newton's method.
    ///
    void solve() override;

    ///
    /// \brief Get all the collision constraints, read only
    ///
    const std::list<std::vector<PbdConstraint*>*>& getConstraintLists() const { return *m_constraintLists; }

    ///
    /// \brief Clear all collision constraints
    ///
    void clearConstraintLists() { m_constraintLists->clear(); }

private:
    size_t m_iterations = 20;                                        ///< Number of NL Gauss-Seidel iterations for regular constraints
    double m_dt = 0.0;                                               ///< time step

    std::shared_ptr<PbdConstraintContainer> m_constraints = nullptr; ///< Vector of constraints

    ///< For quick addition
    std::shared_ptr<std::list<std::vector<PbdConstraint*>*>> m_constraintLists = nullptr;

    PbdState* m_state = nullptr;
    PbdConstraint::SolverType m_solverType = PbdConstraint::SolverType::xPBD;
};
} // namespace imstk