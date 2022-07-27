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
class PbdCollisionConstraint;
class PbdConstraintContainer;

///
/// \struct CollisionConstraintData
///
/// \brief Stores positions and masses of two colliding objects
///
struct CollisionConstraintData
{
    CollisionConstraintData(std::shared_ptr<VecDataArray<double, 3>> posA,
                            std::shared_ptr<DataArray<double>> invMassA,
                            std::shared_ptr<VecDataArray<double, 3>> posB,
                            std::shared_ptr<DataArray<double>> invMassB) :
        m_posA(posA), m_invMassA(invMassA), m_posB(posB), m_invMassB(invMassB)
    {
    }

    std::shared_ptr<VecDataArray<double, 3>> m_posA = nullptr;
    std::shared_ptr<DataArray<double>> m_invMassA   = nullptr;
    std::shared_ptr<VecDataArray<double, 3>> m_posB = nullptr;
    std::shared_ptr<DataArray<double>> m_invMassB   = nullptr;
};

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
    /// \brief Sets the positions the solver should solve with
    ///
    void setPositions(std::shared_ptr<VecDataArray<double, 3>> positions) { this->m_positions = positions; }

    ///
    /// \brief Sets the invMasses the solver should solve with
    ///
    void setInvMasses(std::shared_ptr<DataArray<double>> invMasses) { this->m_invMasses = invMasses; }

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

private:
    size_t m_iterations = 20;                                        ///< Number of NL Gauss-Seidel iterations for regular constraints
    double m_dt;                                                     ///< time step

    std::shared_ptr<PbdConstraintContainer> m_constraints = nullptr; ///< Vector of constraints

    std::shared_ptr<VecDataArray<double, 3>> m_positions = nullptr;
    std::shared_ptr<DataArray<double>>       m_invMasses = nullptr;
    PbdConstraint::SolverType m_solverType = PbdConstraint::SolverType::xPBD;
};

///
/// \class PbdCollisionSolver
///
/// \brief Position Based Dynamics collision solver
/// This solver can sequentially solve constraints in a list
///
class PbdCollisionSolver : SolverBase
{
public:
    PbdCollisionSolver();
    ~PbdCollisionSolver() override = default;

    ///
    /// \brief Get CollisionIterations
    ///
    size_t getCollisionIterations() const { return this->m_collisionIterations; }

    void setCollisionIterations(const size_t iterations) { m_collisionIterations = iterations; }

    ///
    /// \brief Add the global collision contraints to this solver
    ///
    void addCollisionConstraints(std::vector<PbdCollisionConstraint*>* constraints);

    ///
    /// \brief Solve the non linear system of equations G(x)=0 using Newton's method.
    ///
    void solve() override;

private:
    size_t m_collisionIterations = 5;                                                                   ///< Number of NL Gauss-Seidel iterations for collision constraints

    std::shared_ptr<std::list<std::vector<PbdCollisionConstraint*>*>> m_collisionConstraints = nullptr; ///< Collision contraints charged to this solver
};
} // namespace imstk