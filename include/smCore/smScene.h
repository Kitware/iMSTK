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
class smSDK;

struct smSceneLocal
{

public:
    smInt id;
    smSceneLocal()
    {
        sceneUpdatedTimeStamp = 0;
    }

    inline smBool operator ==(smSceneLocal &p_param)
    {
        return id == p_param.id;
    }

    std::vector<smSceneObject*> sceneObjects;
    smUInt sceneUpdatedTimeStamp;
};

///Physics class should have all parameters such as material properties, mesh etc.. for
///note that when you remove the Physics do not delete it.Since propagation of the physics over the
class smScene: public smCoreClass
{
protected:
    smIndiceArray<smLight*> *lights;

private:
    /// \brief number of total objects in the scene
    smInt totalObjects;
    /// \brief error logging
    smErrorLog *log;
    /// \brief scene list lock for thread safe manipulation of the scene
    std::mutex sceneListLock;
    /// \brief reference counter to the scene
    smUInt referenceCounter;
    /// \brief last updated time stampe
    smUInt sceneUpdatedTimeStamp;
    /// \brief scene objects addition queue
    std::vector<smSceneObject*> addQueue;
    smIndiceArray<smSceneLocal*> sceneLocal;
    std::unordered_map<smInt, smInt> sceneLocalIndex;
    /// \brief scene objects storage
    std::vector<smSceneObject*> sceneObjects;
    /// \brief adds the objects in the local scene storage
    void inline copySceneToLocal(smSceneLocal *p_local)
    {
        p_local->sceneObjects.clear();

        for (size_t i = 0; i < sceneObjects.size(); i++)
        {
            p_local->sceneObjects.push_back(sceneObjects[i]);
        }

        p_local->sceneUpdatedTimeStamp = sceneUpdatedTimeStamp;

    }

public:
    /// \brief iterator for scene object. The default iterates over the scene in the order of insertion. For scene graph, this iteration needs to be inherited and modified
    struct smSceneIterator
    {
    protected:
        //smInt startIndex;
        smInt endIndex;
        smInt currentIndex;
        smSceneLocal *sceneLocal;
    public:
        inline smSceneIterator()
        {
            currentIndex = endIndex = 0;
            sceneLocal = NULL;

        }
        /// \brief copy other scene to this current one.
        inline void setScene(smScene *p_scene, smCoreClass *p_core)
        {
            std::lock_guard<std::mutex> lock(p_scene->sceneListLock); //Lock is released when leaves scope
            sceneLocal = p_scene->sceneLocal.getByRef(p_scene->sceneLocalIndex[p_core->uniqueId.ID]);

            if (p_scene->sceneUpdatedTimeStamp > sceneLocal->sceneUpdatedTimeStamp)
            {
                p_scene->copySceneToLocal(sceneLocal);
            }

            endIndex = sceneLocal->sceneObjects.size();
            currentIndex = 0;
        }

        inline smInt start()
        {
            return 0;
        }

        inline smInt end()
        {
            return endIndex;
        }
        /// \brief operator to increment the index to go to the next item in the scene
        inline void operator++()
        {
            currentIndex++;
        }
        /// \brief operator to decrement the index to go to the previous item in the scene
        inline void operator--()
        {
            currentIndex--;
        }
        /// \brief to access a particular entry in the scene with [] notation
        inline smSceneObject* operator[](smInt p_index)
        {
            return sceneLocal->sceneObjects[p_index];
        }
        /// \brief to access a particular entry in the scene with () notation
        inline smSceneObject* operator*()
        {
            return sceneLocal->sceneObjects[currentIndex];
        }
    };

    smScene(smErrorLog *p_log = NULL);
    /// \brief add obejct to the scene, it is thread safe call.

   virtual ~smScene()
   {}
    void registerForScene(smCoreClass *p_simmedtkObject)
    {
        smSceneLocal *local = new smSceneLocal();
        local->id = p_simmedtkObject->uniqueId.ID;
        std::lock_guard<std::mutex> lock(sceneListLock); //Lock is released when leaves scope
        copySceneToLocal(local);
        sceneLocalIndex[p_simmedtkObject->uniqueId.ID] = sceneLocal.checkAndAdd(local);
    }

    ///add physics in the scene
    void  addSceneObject(smSceneObject *p_sceneObject);

    ///remove the phyics in the scene.
    ///The removal of the phsyics in the scene needs some sync all over the modules
    ///so not implemented yet. Be aware that when you remove the phyics do no free the smPhysics class
    void removeSceneObject(smSceneObject *p_sceneObject);

    ///the same as
    void removeSceneObject(smInt p_objectId);

    ///in order to get the phsyics in the scene call this function.
    ///it is thread safe. but it shouldn't be called frequently.
    ///it should be called in the initialization of the viewer, simulation or any other module.
    ///and the the list should be stored internally.
    ///The scene list removal will be taken care of later since the list should be update.
    std::vector<smSceneObject*> getSceneObject();
    /// \brief retursn scene id
    smInt getSceneId();
    /// \brief returns the total number of objects in the scene
    inline smInt getTotalObjects();

    ///Same functionality as addSceneObject
    smScene& operator +=(smSceneObject *p_sceneObject);
    /// \brief add and remove references
    void addRef();
    void removeRef();
    void copySceneObjects(smScene*p_scene);
    smScene &operator =(smScene &p_scene) ;

    /// \brief Initializes lights for rendering
    void initLights();
    /// \brief Enables all currently active lights in the scene
    ///
    /// \detail This should be called in conjunction with disableLights().
    /// Calling this will essentially call glEnable(GL_LIGHT#) for every
    /// enabled light.
    void enableLights();
    /// \brief Disables all lights in the scene
    ///
    /// \detail This should be called in conjunction with enableLights().
    /// Calling this will essentially call glDisable(GL_LIGHT#) for every
    /// light(enabled or not).
    void disableLights();
    /// \brief Place the OpenGL lights in the scene
    ///
    /// \detail Should be called after enableLights()
    void placeLights();
    /// \brief addlight
    smInt addLight(smLight *p_light);
    /// \brief set light given with light ID
    smBool setLight(smInt lightId, smLight *p_light);
    /// \brief refresh lights. updates light  position based on the gl matrix
    void refreshLights();
    /// \brief update light information
    smBool updateLight(smInt p_lightId, smLight *p_light);
    void setLightPos(smInt p_lightId, smLightPos p_pos);
    void setLightPos(smInt p_lightId, smLightPos p_pos, smVec3f p_direction);

    smCamera camera;

    friend class smSDK;
    friend struct smSceneIterator;

    smInt test;
};

#endif
