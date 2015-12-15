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
#include "Core/BaseMesh.h"
#include "Core/ConfigRendering.h"
#include "Mesh/MeshMap.h"
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
    friend class ViewerBase;
    friend class OpenGLViewer;
    friend class Scene;
    friend class ObjectSimulator;

public:
    /// \brief constructor
    SceneObject();

    /// \brief destructor
    ~SceneObject();

    /// \brief Abstract object initialization
    virtual void initialize() = 0;

    virtual bool configure(std::string ConfigFile) = 0;

    /// \brief Load the initial positions, velocities etc.,
    virtual void loadInitialStates() = 0;

    /// \brief attach simulator to the object.
    /// This function that needs to be called to associate the simulator to the object
    virtual void attachObjectSimulator(std::shared_ptr<ObjectSimulator> p_objectSim);

    /// \brief to release the simulator
    virtual void releaseObjectSimulator();

    /// \brief freeze the scene object by stopping any further updates
    void freeze();

    /// \brief activate the scene object
    void activate();

    /// \brief get object simulator
    std::shared_ptr<ObjectSimulator> getObjectSimulator();

    /// \brief attach custome renderer for the object.
    /// If the default rendering is not helpful
    void attachCustomRenderer(std::shared_ptr<CustomRenderer> p_customeRenderer);

    /// \brief release the renderer
    void releaseCustomeRenderer();

    /// \brief returns object id
    int getObjectId();

    /// \brief get unified object id
    UnifiedId::Pointer getObjectUnifiedID();

    std::vector<core::Vec3d> &getLocalVertices();

    ObjectInitFlags &getFlags();

    std::shared_ptr<CustomRenderer> getRenderer();

    ///serialize function explicity writes the object to the memory block
    ///each scene object should know how to write itself to a memory block
    virtual void serialize(void *p_memoryBlock) = 0;

    ///Unserialize function can recover the object from the memory location
    virtual void unSerialize(void *p_memoryBlock) = 0;

    ///this function may not be used
    ///every Scene Object should know how to clone itself. Since the data structures will be
    ///in the beginning of the modules(such as simulator, viewer, collision etc.)
    virtual std::shared_ptr<SceneObject> clone() = 0;

    /// \brief print information related the scene object
    virtual void printInfo() const = 0;

    ///
    /// \brief add one of visual, collision or physics
    /// model to their respective lists
    ///
    bool addModel(std::shared_ptr<Core::BaseMesh>& model, int type);

    ///
    /// \brief add one of physics model to the list
    ///
    bool addPhysicsModel(std::shared_ptr<Core::BaseMesh>& model);

    ///
    /// \brief add one of collision model to the list
    ///
    bool addCollisionModel(std::shared_ptr<Core::BaseMesh>& model);

    ///
    /// \brief add one of visual model to the list
    bool addVisualModel(std::shared_ptr<Core::BaseMesh>& model);

    ///
    /// \brief returns one of visual, collision or physics
    /// models to their respective lists
    ///
    std::shared_ptr<Core::BaseMesh> getModel(const int id, const int type);

    ///
    /// \brief returns a given visual model from the list
    ///
    std::shared_ptr<Core::BaseMesh> getVisualModel(const int id);

    ///
    /// \brief returns a given collision model from the list
    ///
    std::shared_ptr<Core::BaseMesh> getCollisionModel(const int id);

    ///
    /// \brief returns a given physics model from the list
    ///
    std::shared_ptr<Core::BaseMesh> getPhysicsModel(const int id);

    ///
    /// \brief Adds a map after checking if the meshed mapped by the map are
    /// already part of the scene object
    ///
    bool addMap(std::shared_ptr<MeshMap>& map);

    ///
    /// \brief Apply a map with given id from the list
    ///
    virtual void applyMap(int mapID);

    ///
    /// \brief apply all the maps in the list
    ///
    void applyAllMaps();

    //@{

    ///
    /// \brief find if a given mesh is already part of the scene object in either
    /// collision, visual or physics model list
    ///
    bool findMesh(std::shared_ptr<Core::BaseMesh>& m);

    bool findMesh(std::shared_ptr<Core::BaseMesh>& m, const int type);

    //@}

    ///
    /// \brief updates the active collision models at every time step
    ///
    void updateCollisionModels();

    ///
    /// \brief updates the active visual models at every time step
    ///
    void updateVisualModels();

    ///
    /// \brief Embed the internal and external forces into a system of equations
    ///
    virtual std::shared_ptr<systemOfEquations> getInternalAndExternalForces() = 0;

    ///
    /// \brief Return the number of degree of freedom
    ///
    int getNumDof() const
    {
        return numDof;
    }

    /// \brief  returns velocity of at a given location
    /// (not given node) in contact force vector
    virtual core::Vec3d getVelocity(const int) const
    {
        return core::Vec3d::Zero();
    }

    ///
    /// \brief Set all contact forces to zero (if any)
    ///
    void setContactForcesToZero()
    {
        this->fContact.setZero();
    }

    ///
    /// \brief Set contact force at a given degree of freedom
    ///
    void setContactForce(const int dofID, const double &force)
    {
        this->fContact[dofID] = force;
    }

	///
	/// \brief
	///
    const Eigen::VectorXd& SceneObject::getContactForces() const
    {
        return this->fContact;
    }

	///
	/// \brief Set contact forces
	///
    void SceneObject::setContactForces(const Eigen::VectorXd& source)
    {
        this->fContact = source;
    }

    ///
    /// \brief Append contact forces
    ///
    void SceneObject::addContactForces(const Eigen::VectorXd& source)
    {
        this->fContact += source;
    }

private:
    bool isActive;

    int numDof;///> degree of freedom of the object

    // object simulator that will simulate the object
    std::shared_ptr<ObjectSimulator> objectSim;

    std::shared_ptr<CustomRenderer> customRender;

    std::vector<core::Vec3d> localVertices; //!< local copy of vertices

    ObjectInitFlags flags;

    bool hasContactForces;

    Eigen::VectorXd fContact;

    //-------------------------------------------------------------------
    // Geometric models and maps
    //-------------------------------------------------------------------

    // List of maps between models
    std::vector<std::shared_ptr<MeshMap>> maps;

    // List of discretized geometric models used for collision
    std::vector<std::shared_ptr<Core::BaseMesh>> collisionModels;

    // List of discretized geometric models used for visual rendering
    std::vector<std::shared_ptr<Core::BaseMesh>> visualModels;

    // List of discretized geometric models used for physics-based computations
    std::vector<std::shared_ptr<Core::BaseMesh>> physicsModels;

    //-------------------------------------------------------------------
};

#endif
