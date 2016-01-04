// This file is part of the SimMedTK project.
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
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#ifndef SMVEGAFEMSCENEOBJECT_DEFORMABLE_H
#define SMVEGAFEMSCENEOBJECT_DEFORMABLE_H

// SimMedTK includes
#include "Core/SceneObject.h"
#include "TimeIntegrators/TimeIntegrator.h"
#include "TimeIntegrators/BackwarEuler.h"
#include "TimeIntegrators/ForwardEuler.h"

///
/// \brief Base class for all deformable scene objects.
///
class DeformableSceneObject : public SceneObject, public OdeSystem
{
public:
    ///
    /// \brief Constructor
    ///
    DeformableSceneObject() :
        OdeSystem(),
        integrationScheme(TimeIntegrator::ImplicitEuler)
    {
    }

    ///
    /// \brief Destructor
    ///
    virtual ~DeformableSceneObject() = default;

    ///
    /// \brief Append the contact forces (if any) to external forces
    ///
    void applyContactForces()
    {
        for(const auto &cf : this->getContactForces())
        {
            int i = cf.first;
            f(i) += cf.second(0);
            f(i+1) += cf.second(1);
            f(i+2) += cf.second(2);
        }
    }

    ///
    /// \brief Set the integration scheme used to solve the ODE system.
    ///
    void setTimeIntegrator(TimeIntegrator::IntegratorType integrator)
    {
        this->integrationScheme = integrator;
    }

    ///
    /// \brief Initialize the ode solver.
    ///
    bool init()
    {
        auto thisPointer = this->safeDownCast<DeformableSceneObject>();
        switch(integrationScheme)
        {
            case TimeIntegrator::ImplicitEuler:
            {
                this->odeSolver = std::make_shared<BackwardEuler>(thisPointer);
            }
            case TimeIntegrator::ExplicitEuler:
            {
                this->odeSolver = std::make_shared<ForwardEuler>(thisPointer);
            }
            default:
            {
                std::cerr << "Invalid time integration scheme." << std::endl;
            }

            return false;
        }
    }

    ///
    /// \brief Update states
    ///
    void update(double dt)
    {
        if(!this->odeSolver)
        {
            std::cerr << "Ode solver needs to be set." << std::endl;
            return;
        }

        this->odeSolver->solve(*this->currentState,*this->newState,dt);

        this->currentState.swap(this->previousState);
        this->currentState.swap(this->newState);

        // TODO: Check state validity
    }

    ///
    /// \brief Reset the current state to the initial state
    ///
    virtual void resetToInitialState()
    {
        *this->currentState = *this->initialState;
        *this->previousState = *this->initialState;
    }

    ///
    /// \brief Return the current state.
    ///
    std::shared_ptr<OdeSystemState> getCurrentState()
    {
        this->currentState;
    }

    ///
    /// \brief Return the previous state.
    ///
    std::shared_ptr<OdeSystemState> getPreviousState()
    {
        this->previousState;
    }

protected:
    std::shared_ptr<TimeIntegrator> odeSolver; ///> Integration scheme

    // Consecutive system states
    std::shared_ptr<OdeSystemState> currentState; ///> Current model state
    std::shared_ptr<OdeSystemState> previousState;///> Previous model state
    std::shared_ptr<OdeSystemState> newState;     ///> Storage for the next state

    core::SparseMatrixd M; ///> Mass matrix
    core::SparseMatrixd C; ///> Raleigh Damping matrix
    core::SparseMatrixd D; ///> Raleigh Damping matrix
    core::SparseMatrixd K; ///> Stiffness matrix
    core::Vectord f;       ///> Accumulative forces vector

    TimeIntegrator::IntegratorType integrationScheme; ///> Integration scheme used.
};

#endif // SMVEGAFEMSCENEOBJECT_DEFORMABLE_H
