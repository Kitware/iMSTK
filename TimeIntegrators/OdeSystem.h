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

namespace imstk {

///
/// \brief Ode system of equations. Represent a ODE system of equations written as
///     dv/dt=f(t,x,v)
///     dx/dt=v, with initial conditions x(0)=x0; v(0)=v0.
///
class OdeSystem : public LinearSystem<SparseMatrixd>
{
public:
    using BaseSystem = LinearSystem<SparseMatrixd>;
    using FunctionOperatorType =
        std::function<const Vectord&(const OdeSystemState &)>;
    using JacobianOperatorType =
        std::function<const SparseMatrixd&(const OdeSystemState &)>;

public:
    OdeSystem();
    virtual ~OdeSystem() = default;

    ///
    /// \brief Set the derivative with respect to v of the right hand side.
    ///
    /// \param newDFv Derivative function. Returns a sparse matrix.
    ///
    void setJaconbianFv(JacobianOperatorType newDFv);

    ///
    /// \brief Set the derivative with respect to x of the right hand side.
    ///
    /// \param newDFx Derivative function. Returns a sparse matrix.
    ///
    void setJaconbianFx(JacobianOperatorType newDFx);

    ///
    /// \brief Set the mass matrix evaluation function.
    ///
    /// \param newMass New mass function. Returns a sparse matrix.
    ///
    void setMass(JacobianOperatorType newMass);

    ///
    /// \brief Set the Lagrangian damping matrix evaluation function.
    ///
    /// \param newDamping New damping function. Returns a sparse matrix.
    ///
    void setDamping(JacobianOperatorType newDamping);

    ///
    /// \brief Set the right hand side evaluation function.
    ///
    /// \param newF New rhs function. Returns a vector.
    ///
    void setRHSFunction(FunctionOperatorType newF);

    ///
    /// \brief Evaluate -df/dx function at specified argument.
    ///
    /// \param state Current position and velocity.
    ///
    const SparseMatrixd &evalDFx(const OdeSystemState &state);

    ///
    /// \brief Evaluate -df/dv function at specified argument.
    ///
    /// \param state Current position and velocity.
    ///
    const SparseMatrixd &evalDFv(const OdeSystemState &state);

    ///
    /// \brief Evaluate mass function at specified argument.
    ///
    /// \param state Current position and velocity.
    ///
    const SparseMatrixd &evalMass(const OdeSystemState &state);

    ///
    /// \brief Evaluate rhs function at specified argument.
    ///
    /// \param state Current position and velocity.
    ///
    const Vectord &evalRHS(const OdeSystemState &state);

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

private:
    JacobianOperatorType DFx; ///> Function to evaluate -dF/dx, required for implicit time stepping schemes.
    JacobianOperatorType DFv; ///> Function to evaluate -dF/dv, required for implicit time stepping schemes.
    JacobianOperatorType Mass; ///> Function to evaluate the mass matrix.
    JacobianOperatorType Damping; ///> Additional damping matrix.
    FunctionOperatorType F; ///> ODE right hand side function

    SparseMatrixd systemMatrix;   ///> Linear system matrix storage.
    Vectord rhs;                  ///> Right hand side vector storage.

protected:
    std::shared_ptr<OdeSystemState> initialState; ///> Initial state of the system.
};

/// Inlined functions

//---------------------------------------------------------------------------
inline const SparseMatrixd &OdeSystem::evalDFx(const OdeSystemState &state)
{
    if(!this->DFx)
    {
        /// Log this
    }
    return this->DFx(state);
}

//---------------------------------------------------------------------------
inline const SparseMatrixd &OdeSystem::evalDFv(const OdeSystemState &state)
{
    if(!this->DFv)
    {
        /// Log this
    }
    return this->DFv(state);
}

//---------------------------------------------------------------------------
inline const SparseMatrixd &OdeSystem::evalMass(const OdeSystemState &state)
{
    if(!this->Mass)
    {
        /// Log this
    }
    return this->Mass(state);
}

//---------------------------------------------------------------------------
inline const Vectord &OdeSystem::evalRHS(const OdeSystemState &state)
{
    if(!this->F)
    {
        /// Log this
    }
    return this->F(state);
}

}

#endif // ODESYSTEM_H
