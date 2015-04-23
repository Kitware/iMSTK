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

#ifndef SMSCENE_H
#define SMSCENE_H

// STL includes
#include <memory>
#include <mutex>
#include <unordered_map>

// SimMedTK includes
#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smCore/smSceneObject.h"
#include "smCore/smErrorLog.h"
#include "smUtilities/smDataStructures.h"
#include "smCore/smDoubleBuffer.h"
#include "smRendering/smLight.h"
#include "smRendering/smCamera.h"

class smPipe;
class smScene;
class smSDK;

struct smSceneLocal
{

public:
    smInt id;
    smSceneLocal()
    {
        sceneUpdatedTimeStamp = 0;
    }

    smBool operator ==(smSceneLocal &p_param)
    {
        return id == p_param.id;
    }

    std::vector<std::shared_ptr<smSceneObject>> sceneObjects;
    smUInt sceneUpdatedTimeStamp;
};


/// \brief iterator for scene object. The default iterates over the scene in the order of insertion. For scene graph, this iteration needs to be inherited and modified
class smSceneIterator
{
public:
    smSceneIterator();
    /// \brief copy other scene to this current one.
    void setScene(std::shared_ptr<smScene> p_scene, std::shared_ptr<smCoreClass> p_core);

    smInt start();

    smInt end();

    /// \brief operator to increment the index to go to the next item in the scene
    void operator++();

    /// \brief operator to decrement the index to go to the previous item in the scene
    void operator--();

    /// \brief to access a particular entry in the scene with [] notation
    std::shared_ptr<smSceneObject> operator[](smInt p_index);

    /// \brief to access a particular entry in the scene with () notation
    std::shared_ptr<smSceneObject> operator*();

private:
    smInt endIndex;
    smInt currentIndex;
    std::shared_ptr<smSceneLocal> sceneLocal;
};

///Physics class should have all parameters such as material properties, mesh etc.. for
///note that when you remove the Physics do not delete it.Since propagation of the physics over the
class smScene: public smCoreClass
{
public:
    smScene(std::shared_ptr<smErrorLog> p_log = nullptr);

    virtual ~smScene(){}

    /// \brief add obejct to the scene, it is thread safe call.
    void registerForScene(std::shared_ptr<smCoreClass> p_simmedtkObject);

    ///add physics in the scene
    void  addSceneObject(std::shared_ptr<smSceneObject> p_sceneObject);

    ///remove the phyics in the scene.
    ///The removal of the phsyics in the scene needs some sync all over the modules
    ///so not implemented yet. Be aware that when you remove the phyics do no free the smPhysics class
    void removeSceneObject(std::shared_ptr<smSceneObject> p_sceneObject);

    ///the same as
    void removeSceneObject(std::shared_ptr<smUnifiedId> p_objectId);

    ///in order to get the phsyics in the scene call this function.
    ///it is thread safe. but it shouldn't be called frequently.
    ///it should be called in the initialization of the viewer, simulation or any other module.
    ///and the the list should be stored internally.
    ///The scene list removal will be taken care of later since the list should be update.
    std::vector<std::shared_ptr<smSceneObject>> &getSceneObject();

    /// \brief retursn scene id
    std::shared_ptr<smUnifiedId> getSceneId();

    /// \brief returns the total number of objects in the scene
    inline smInt getTotalObjects();

    ///Same functionality as addSceneObject
    std::shared_ptr<smScene> operator+=(std::shared_ptr<smSceneObject> p_sceneObject);

    /// \brief add and remove references
    void addRef();

    void removeRef();

    void copySceneObjects(std::shared_ptr<smScene> p_scene);

    std::shared_ptr<smScene> operator=(std::shared_ptr<smScene> p_scene);

    /// \brief Initializes lights for rendering
    void initLights();

    /// \brief  enable attached lights
    void enableLights();

    /// \brief addlight
    smInt addLight(std::shared_ptr<smLight> p_light);

    /// \brief set light given with light ID
    smBool setLight(smInt lightId, std::shared_ptr<smLight> p_light);

    /// \brief refresh lights. updates light  position based on the gl matrix
    void refreshLights();

    /// \brief update light information
    smBool updateLight(smInt p_lightId, std::shared_ptr<smLight> p_light);

    void setLightPos(smInt p_lightId, smLightPos p_pos);

    void setLightPos(smInt p_lightId, smLightPos p_pos, smVec3d p_direction);

    std::shared_ptr<smCamera> getCamera()
    {
        return camera;
    }

    void addCamera(std::shared_ptr<smCamera> sceneCamera)
    {
        camera = sceneCamera;
    }

private:
    /// \brief adds the objects in the local scene storage
    void inline copySceneToLocal(std::shared_ptr<smSceneLocal> p_local);

protected:
    smIndiceArray<std::shared_ptr<smLight>> *lights;

private:
    smIndiceArray<std::shared_ptr<smSceneLocal>> sceneLocal;
    std::vector<std::shared_ptr<smSceneObject>> addQueue;       // scene objects addition queue
    std::vector<std::shared_ptr<smSceneObject>> sceneObjects;   // scene objects storage
    std::unordered_map<smInt, smInt> sceneLocalIndex;
    std::shared_ptr<smErrorLog> log;                            // error logging
    std::mutex sceneListLock;                                   // scene list lock for thread safe manipulation of the scene

    smUInt sceneUpdatedTimeStamp;                               // last updated time stampe
    smUInt referenceCounter;                                    // reference counter to the scene
    smInt totalObjects;                                         // number of total objects in the scene
    std::shared_ptr<smCamera> camera;

    friend class smSDK;

    friend struct smSceneIterator;

    smInt test;
};

#endif
