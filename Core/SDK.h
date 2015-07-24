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

#ifndef SMSDK_H
#define SMSDK_H

// SimMedTK includes
#include "Core/Config.h"
#include "Core/CoreClass.h"
#include "Core/Scene.h"
#include "Core/Simulator.h"
#include "Core/Module.h"
#include "Core/DataStructures.h"
#include "Core/MakeUnique.h"
#include "Rendering/Viewer.h"

/// \brief maximum entities in the framework
#define SIMMEDTK_SDK_MAXMESHES 100
#define SIMMEDTK_SDK_MAXMODULES 100
#define SIMMEDTK_SDK_MAXOBJECTSIMULATORS 100
#define SIMMEDTK_SDK_MAXSCENES 100
#define SIMMEDTK_SDK_MAXSCENEOBJTECTS 100

template<typename T> class IndiceArray;
class Scene;

/// \brief SDK class. it is a singlenton class for each machine runs the framework
class SDK: public CoreClass
{
public:
    /// \brief destructor
    ~SDK();

    /// \brief update scene list. not implemented
    void updateSceneListAll();

    void initRegisteredModules();
    void runRegisteredModules();
    void shutDown();

    ///for now both functions below are the same. But it maybe subject to change.
    static std::shared_ptr<SDK> createSDK();

    ///Creates the sdk, viewer and scene 0
    static std::shared_ptr<SDK> createStandardSDK();

    static std::shared_ptr<SDK> getInstance();

    /// \brief Registers a viewer object with the SDK
    ///
    void addViewer(std::shared_ptr<ViewerBase> p_viewer);

    /// \brief Creates and registers a viewer object with the SDK
    ///
    std::shared_ptr<ViewerBase> createViewer();

    ///SDK returns a pointer to the viewer
    std::shared_ptr<ViewerBase> getViewerInstance();

    ///SDK creates simualtor
    std::shared_ptr<Simulator> createSimulator();

    ///SDK creates simualtor
    std::shared_ptr<Simulator> getSimulator();

    ///SDK creates scene
    std::shared_ptr<Scene> createScene();

    ///SDK creates scene
    std::shared_ptr<Scene> getScene(size_t sceneId)
    {
        return sceneList.at(sceneId);
    }

    ///SDK returns logger for the system
    std::shared_ptr<ErrorLog> getErrorLog();

    ///terminates every module. Do it later on with Messager
    void terminateAll();

    ///release the scene from the SDK..not implemented yet
    void releaseScene(std::shared_ptr<Scene> scene);

    /// Prevent use of assignament operator
    SDK &operator=(const SDK&) = delete;

    /// \brief run the SDK
    void run();

    /// \brief add reference to a core class
    void addRef(std::shared_ptr<CoreClass> p_coreClass);

    /// \brief removes reference on core class
    void removeRef(std::shared_ptr<CoreClass> p_coreClass);

    /// \brief register functions
    void registerMesh(std::shared_ptr<BaseMesh> p_mesh);

    void registerModule(std::shared_ptr<Module> p_mod);

    void registerObjectSimulator(std::shared_ptr<ObjectSimulator> p_os);

    void registerCollisionDetection(std::shared_ptr<CollisionDetection> p_col);

    void registerScene(std::shared_ptr<Scene> p_sc);

    void registerSceneObject(std::shared_ptr<SceneObject> p_sco);

    void addSceneActor(std::shared_ptr<SceneObject> p_sco, std::shared_ptr<ObjectSimulator> p_os, int p_scId=0);

private:
    static std::once_flag sdkCallOnceFlag;

    bool shutdown; ///< Tells the SDK to terminate
    int sceneIdCounter; ///< this id is incremented when a scene is created
    int argc;
    char argv;
    bool isModulesStarted;

    std::shared_ptr<ErrorLog> errorLog; ///< error log
    std::shared_ptr<ViewerBase> viewer; ///< Reference to the sdk viewer object
    std::shared_ptr<Simulator> simulator; ///< Reference to the sdk simulator object

    ///holds the references to the entities in the framework
    std::vector<std::shared_ptr<BaseMesh>> meshList;
    std::vector<std::shared_ptr<Module>> moduleList;
    std::vector<std::shared_ptr<ObjectSimulator>> simulatorList;
    std::vector<std::shared_ptr<CollisionDetection>> collisionDetectionList;
    std::vector<std::shared_ptr<Scene>> sceneList; ///< scene list
    std::vector<std::shared_ptr<SceneObject>> sceneObjectList;

    std::vector<std::thread> modules; ///< Stores a list of running module threads
private:
    /// \brief constructor
    SDK();
};

#endif
