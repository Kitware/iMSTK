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

#ifndef DEFORMABLESCENEOBJECT_H
#define DEFORMABLESCENEOBJECT_H

// SimMedTK includes
#include "SceneModels/SceneObject.h"
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
    DeformableSceneObject();

    ///
    /// \brief Destructor
    ///
    virtual ~DeformableSceneObject() = default;

    ///
    /// \brief Append the contact forces (if any) to external forces
    ///
    void applyContactForces();

    ///
    /// \brief Set the integration scheme used to solve the ODE system.
    ///
    void setTimeIntegrator(TimeIntegrator::IntegratorType integrator);

    ///
    /// \brief Initialize the ode solver.
    ///
    void initialize() override;

    ///
    /// \brief Update states
    ///
    void update(const double dt) override;

    ///
    /// \brief Update states
    ///
    virtual void updateMesh() = 0;

    ///
    /// \brief Reset the current state to the initial state
    ///
    virtual void resetToInitialState();

    ///
    /// \brief Return the current state.
    ///
    std::shared_ptr<OdeSystemState> getCurrentState();

    ///
    /// \brief Return the previous state.
    ///
    std::shared_ptr<OdeSystemState> getPreviousState();

    ///
    /// \brief Returns velocity of at a given location for the current state.
    ///
    Eigen::Map<core::Vec3d> getVelocity(const int index)
    {
        auto velocities = this->currentState->getVelocities();
        return Eigen::Map<core::Vec3d>(&velocities(index));
    }

private:
    ///////////////////////////////////////////////////////////////////////////////
    //////////// TODO: These are pure virtual methods from superclass. ////////////
    ////////////    They should be removed in the future.              ////////////
    ///////////////////////////////////////////////////////////////////////////////

    ///serialize function explicity writes the object to the memory block
    ///each scene object should know how to write itself to a memory block
    void serialize(void *){}

    ///Unserialize function can recover the object from the memory location
    void unSerialize(void *){};

    ///this function may not be used
    ///every Scene Object should know how to clone itself. Since the data structures will be
    ///in the beginning of the modules(such as simulator, viewer, collision etc.)
    std::shared_ptr<SceneObject> clone(){return nullptr;};

    /// \brief print information related the scene object
    void printInfo() const{};

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

#endif // DEFORMABLESCENEOBJECT_H
