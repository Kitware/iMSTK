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

#ifndef TIME_INTEGRATORS_ODESYSTEM_H
#define TIME_INTEGRATORS_ODESYSTEM_H

#include "TimeIntegrators/OdeSystemState.h"
#include "Solvers/SystemOfEquations.h"
#include "Core/Matrix.h"

///
/// \brief Ode system of equations. Represent a ODE system of equations written as
///     dv/dt=f(t,x,v)
///     dx/dt=v, with initial conditions x(0)=x0; v(0)=v0.
///
class OdeSystem
{
public:
    using MatrixFunctionType = std::function<const core::SparseMatrixd&(const OdeSystemState &state)>;
    using FunctionType = std::function<const core::Vectord&(const OdeSystemState &state)>;

public:
    OdeSystem() = default;
    ~OdeSystem() = default;

    ///
    /// \brief Set the derivative with respect to v of the right hand side.
    ///
    /// \param newDFv Derivative function. Returns a sparse matrix.
    ///
    void setJaconbianFv(MatrixFunctionType newDFv);

    ///
    /// \brief Set the derivative with respect to x of the right hand side.
    ///
    /// \param newDFx Derivative function. Returns a sparse matrix.
    ///
    void setJaconbianFx(MatrixFunctionType newDFx);

    ///
    /// \brief Set the mass matrix evaluation function.
    ///
    /// \param newMass New mass function. Returns a sparse matrix.
    ///
    void setMass(MatrixFunctionType newMass);

    ///
    /// \brief Set the Lagrangian damping matrix evaluation function.
    ///
    /// \param newDamping New damping function. Returns a sparse matrix.
    ///
    void setDamping(MatrixFunctionType newDamping);

    ///
    /// \brief Set the right hand side evaluation function.
    ///
    /// \param newF New rhs function. Returns a vector.
    ///
    void setFunction(FunctionType newF);

    ///
    /// \brief Evaluate -df/dx function at specified argument.
    ///
    /// \param state Current position and velocity.
    ///
    const core::SparseMatrixd &evalDFx(const OdeSystemState &state);

    ///
    /// \brief Evaluate -df/dv function at specified argument.
    ///
    /// \param state Current position and velocity.
    ///
    const core::SparseMatrixd &evalDFv(const OdeSystemState &state);

    ///
    /// \brief Evaluate mass function at specified argument.
    ///
    /// \param state Current position and velocity.
    ///
    const core::SparseMatrixd &evalMass(const OdeSystemState &state);

    ///
    /// \brief Evaluate rhs function at specified argument.
    ///
    /// \param state Current position and velocity.
    ///
    const core::Vectord &evalF(const OdeSystemState &state);

    ///
    /// \brief Get the initial velocities and positions of the system.
    ///
    /// \return Pinter to initial state.
    ///
    const std::shared_ptr<OdeSystemState> getInitialState() const;

    ///
    /// \brief Set the initial velocities and positions of the system.
    ///
    /// \param newState New positions and velocities.
    ///
    void setInitialState(std::shared_ptr<OdeSystemState> newState);

    ///
    /// \brief Compute and store the system matrix for implicit integration scheme.
    ///
    /// \param state Current state
    /// \param newState New state
    /// \param timeStep Time step used to discretize the ODE.
    ///
    virtual void computeImplicitSystemLHS(const OdeSystemState &state,
                                          OdeSystemState &newState,
                                          const double timeStep,
                                          bool computeRHS = true);

    ///
    /// \brief Compute and store the system matrix for explicit integration scheme.
    ///
    /// \param state Current state
    /// \param newState New state
    /// \param timeStep Time step used to discretize the ODE.
    ///
    virtual void computeExplicitSystemLHS(const OdeSystemState &state,
                                          OdeSystemState &newState,
                                          const double timeStep,
                                          bool computeRHS = true);

    ///
    /// \brief Compute and store the right hand side of the system for implicit
    /// \integration scheme.
    ///
    /// \param state Current state
    /// \param newState New state
    /// \param timeStep Time step used to discretize the ODE.
    ///
    virtual void computeImplicitSystemRHS(const OdeSystemState &state,
                                          OdeSystemState &newState,
                                          double timeStep);

    ///
    /// \brief Get the system matrix corresponding to this ODE system.
    ///
    /// \return Sparse matrix constant reference.
    ///
    const core::SparseMatrixd &getSystemMatrix() const;

    ///
    /// \brief Get the system matrix corresponding to this ODE system.
    ///
    /// \return Sparse matrix constant reference.
    ///
    core::SparseMatrixd &getSystemMatrix();

    ///
    /// \brief Set the system matrix corresponding to this ODE system.
    ///
    /// \param newMatrix New matrix.
    ///
    void setSystemMatrix(const core::SparseMatrixd &newMatrix);

    ///
    /// \brief Get the system rhs corresponding to this ODE system.
    ///
    /// \return Vector constant reference.
    ///
    const core::Vectord &getRHS() const;

    ///
    /// \brief Get the system rhs corresponding to this ODE system.
    ///
    /// \return Vector reference.
    ///
    core::Vectord &getRHS();

    ///
    /// \brief Set the system rhs corresponding to this ODE system.
    ///
    /// \param newRhs new rhs.
    ///
    void setRHS(const core::Vectord &newRhs);

private:
    MatrixFunctionType DFx; ///> Function to evaluate -dF/dx, required for implicit time stepping schemes.
    MatrixFunctionType DFv; ///> Function to evaluate -dF/dv, required for implicit time stepping schemes.
    MatrixFunctionType Mass; ///> Function to evaluate the mass matrix.
    MatrixFunctionType Damping; ///> Additional damping matrix.
    FunctionType F; ///> ODE right hand side function

    core::SparseMatrixd systemMatrix;   ///> Linear system matrix storage.
    core::Vectord rhs;                  ///> Right hand side vector storage.

protected:
    std::shared_ptr<OdeSystemState> initialState; ///> Initial state of the system.
};

/// Inlined functions

//---------------------------------------------------------------------------
inline const core::SparseMatrixd &OdeSystem::evalDFx(const OdeSystemState &state)
{
    if(!this->DFx)
    {
        /// Log this
    }
    return this->DFx(state);
}

//---------------------------------------------------------------------------
inline const core::SparseMatrixd &OdeSystem::evalDFv(const OdeSystemState &state)
{
    if(!this->DFv)
    {
        /// Log this
    }
    return this->DFv(state);
}

//---------------------------------------------------------------------------
inline const core::SparseMatrixd &OdeSystem::evalMass(const OdeSystemState &state)
{
    if(!this->Mass)
    {
        /// Log this
    }
    return this->Mass(state);
}

//---------------------------------------------------------------------------
inline const core::Vectord &OdeSystem::evalF(const OdeSystemState &state)
{
    if(!this->F)
    {
        /// Log this
    }
    return this->F(state);
}

#endif // ODESYSTEM_H
