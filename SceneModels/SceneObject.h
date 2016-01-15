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

#ifndef SMSCENEOBJECT_H
#define SMSCENEOBJECT_H

// STL includes
#include <vector>
#include <memory>
#include <unordered_map>

// SimMedTK includes
#include "Core/Vector.h"
#include "Core/Config.h"
#include "Core/CoreClass.h"
#include "Core/ConfigRendering.h"
#include "Rendering/CustomRenderer.h"

//forward class declaration
class ObjectSimulator;
class CustomRenderer;

///booleans for objects indicate whether they're initialized or not.
struct ObjectInitFlags
{
    bool isViewerInit;
    bool isSimulatorInit;

};

/// \brief Base class for any object in the scene both
/// physical and otherwise
class SceneObject : public CoreClass
{
    friend class SDK;

public:
    ///
    /// \brief Constructor.
    ///
    SceneObject();

    ///
    /// \brief Destructor.
    ///
    ~SceneObject() = default;

    ///
    /// \brief Abstract object initialization.
    ///
    virtual void initialize() = 0;

    ///
    /// \brief Load the initial posiitons, velocities etc.,
    ///
    virtual void loadInitialStates() = 0;

    ///
    /// \brief serialize function explicity writes the object to the memory block
    ///     each scene object should know how to write itself to a memory block.
    ///
    virtual void serialize(void *p_memoryBlock) = 0;

    ///
    /// \brief Unserialize function can recover the object from the memory location.
    ///
    virtual void unSerialize(void *p_memoryBlock) = 0;

    ///
    /// \brief This function may not be used
    ///     every Scene Object should know how to clone itself. Since the data structures
    ///     will be in the beginning of the modules(such as simulator, viewer,
    ///     collision etc.)
    ///
    virtual std::shared_ptr<SceneObject> clone() = 0;

    ///
    /// \brief print information related the scene object.
    ///
    virtual void printInfo() const = 0;

    ///
    /// \brief Define this function if you want to configure this scene model with using
    ///     an external file.
    ///
    virtual bool configure(const std::string &/*ConfigFile*/);

    ///
    /// \brief Attach simulator to the object.
    ///     This function that needs to be called to associate the simulator
    ///     to the object.
    ///
    virtual void attachObjectSimulator(std::shared_ptr<ObjectSimulator> p_objectSim);

    ///
    /// \brief To release the simulator.
    ///
    virtual void releaseObjectSimulator();

    ///
    /// \brief Freeze the scene object by stopping any further updates.
    ///
    void freeze();

    ///
    /// \brief Get object simulator.
    ///
    std::shared_ptr<ObjectSimulator> getObjectSimulator();

    ///
    /// \brief Returns object id.
    ///
    int getObjectId();

    ///
    /// \brief Get unified object id.
    ///
    UnifiedId::Pointer getObjectUnifiedID();

    ///
    /// \brief Get local initialization flags.
    ///
    ObjectInitFlags &getFlags();

    ///
    /// \brief Set to activate this scene model.
    ///
    void activate();

    ///
    /// \brief Set to compute contact forces.
    ///
    bool computeContactForce();

    ///
    /// \brief Set to not compute contact forces.
    ///
    void setContactForcesOff();

    ///
    /// \brief Set to not compute contact forces.
    ///
    void setContactForcesOn();

    ///
    /// \brief Get contact forces vector.
    ///
    std::unordered_map<int,core::Vec3d> &getContactForces();

    ///
    /// \brief Get the map of contact forces.
    ///
    /// \return Map containing indices with contact points.
    ///
    const std::unordered_map<int,core::Vec3d> &getContactForces() const;

    ///
    /// \brief Get contact forces vector
    ///
    std::unordered_map<int,core::Vec3d> &getContactPoints();

    ///
    /// \brief Get contact forces vector
    ///
    const std::unordered_map<int,core::Vec3d> &getContactPoints() const;

    ///
    /// \brief Returns velocity of at a given location
    ///     (not given node) in contact force vector
    ///
    virtual Eigen::Map<core::Vec3d> getVelocity(const int);

    ///
    /// \brief Set all contact forces to zero (if any)
    ///
    void setContactForcesToZero();

    ///
    /// \brief Get contact forces vector
    ///
    void setContactForce(const int dofID, const core::Vec3d &force);

    ///
    /// \brief Get contact forces vector
    ///
    void setContactForce(const int dofID,
                         const core::Vec3d &point,
                         const core::Vec3d &force);

    ///
    /// \brief Get contact forces vector
    ///
    void setModel(std::shared_ptr<Model> m);

    ///
    /// \brief Get contact forces vector
    ///
    std::shared_ptr<Model> getModel();

    ///
    /// \brief Get contact forces vector
    ///
    virtual void update(const double /*dt*/);

protected:
    bool hasContactForces;
    bool isActive;
    ObjectInitFlags flags;
    std::shared_ptr<Model> model; //!< model attached to this scene object
    std::shared_ptr<ObjectSimulator> objectSim; //!< object simulator that will simulate the object
    std::unordered_map<int,core::Vec3d> contactForces;
    std::unordered_map<int,core::Vec3d> contactPoints;
};

#endif
