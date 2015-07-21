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

// STD includes
#include <vector>
#include <memory>

// SimMedTK includes
#include "Core/Vector.h"
#include "Core/Config.h"
#include "Core/CoreClass.h"
#include "Rendering/ConfigRendering.h"
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
    friend class Viewer;
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

    /// \brief Load the initial posiitons, velocities etc.,
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

    core::StdVector3d &getLocalVertices();

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

protected:
    bool isActive;

private:
    std::shared_ptr<ObjectSimulator> objectSim; // object simulator that will simulate the object
    std::shared_ptr<CustomRenderer> customRender;
    core::StdVector3d localVertices; // local copy of vertices
    ObjectInitFlags flags;
};

#endif
