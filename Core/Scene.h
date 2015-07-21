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
#include "Core/Config.h"
#include "Core/CoreClass.h"
#include "Core/ErrorLog.h"
#include "Core/Light.h"
#include "Core/SceneObject.h"
#include "Core/DataStructures.h"
#include "Event/KeyboardEvent.h"
#include "Event/MouseButtonEvent.h"
#include "Event/MouseMoveEvent.h"
#include "Rendering/Camera.h"

class Scene;
class SDK;

struct SceneLocal
{

public:
    int id;
    SceneLocal()
    {
        sceneUpdatedTimeStamp = 0;
    }

    bool operator ==(SceneLocal &p_param)
    {
        return id == p_param.id;
    }

    std::vector<std::shared_ptr<SceneObject>> sceneObjects;
    unsigned int sceneUpdatedTimeStamp;
};

///Physics class should have all parameters such as material properties, mesh etc.. for
///note that when you remove the Physics do not delete it.Since propagation of the physics over the
class Scene: public CoreClass
{
public:
    Scene(std::shared_ptr<ErrorLog> p_log = nullptr);

    virtual ~Scene(){}

    ///add physics in the scene
    void  addSceneObject(std::shared_ptr<SceneObject> p_sceneObject);

    ///remove the phyics in the scene.
    ///The removal of the phsyics in the scene needs some sync all over the modules
    ///so not implemented yet. Be aware that when you remove the phyics do no free the Physics class
    void removeSceneObject(std::shared_ptr<SceneObject> p_sceneObject);

    ///the same as
    void removeSceneObject(std::shared_ptr<UnifiedId> p_objectId);

    ///in order to get the phsyics in the scene call this function.
    ///it is thread safe. but it shouldn't be called frequently.
    ///it should be called in the initialization of the viewer, simulation or any other module.
    ///and the the list should be stored internally.
    ///The scene list removal will be taken care of later since the list should be update.
    std::vector<std::shared_ptr<SceneObject>> &getSceneObject();

    /// \brief retursn scene id
    std::shared_ptr<UnifiedId> getSceneId();

    ///Same functionality as addSceneObject
    std::shared_ptr<Scene> operator+=(std::shared_ptr<SceneObject> p_sceneObject);

    void copySceneObjects(std::shared_ptr<Scene> p_scene);

    std::shared_ptr<Scene> operator=(std::shared_ptr<Scene> p_scene);

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
    int addLight(std::shared_ptr<Light> p_light);

    /// \brief refresh lights. updates light  position based on the gl matrix
    void refreshLights();

    void setLightPos(int p_lightId, LightPos p_pos);

    void setLightPos(int p_lightId, LightPos p_pos, core::Vec3d p_direction);

    std::shared_ptr<Camera> getCamera()
    {
        return camera;
    }

    void addCamera(std::shared_ptr<Camera> sceneCamera)
    {
        camera = sceneCamera;
    }

    /// \brief adds the objects in the local scene storage
    void copySceneToLocal(SceneLocal &p_local);

private:
    std::shared_ptr<Camera> camera;                           //Camera for the scene
    std::vector<std::shared_ptr<Light> > lights;              //Lights in the scene
    std::vector<std::shared_ptr<SceneObject>> sceneObjects;   // scene objects storage
    std::shared_ptr<ErrorLog> log;                            // error logging
    std::mutex sceneLock;                                       // scene list lock for thread safe manipulation of the scene
    unsigned int sceneUpdatedTimeStamp;                               // last updated time stamp
};

#endif
