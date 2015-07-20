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
#include "Config.h"
#include "CoreClass.h"
#include "Scene.h"
#include "Rendering/Viewer.h"
#include "Simulator.h"
#include "Module.h"
#include "DataStructures.h"
#include "MakeUnique.h"

/// \brief maximum entities in the framework
#define SIMMEDTK_SDK_MAXMESHES 100
#define SIMMEDTK_SDK_MAXMODULES 100
#define SIMMEDTK_SDK_MAXOBJECTSIMULATORS 100
#define SIMMEDTK_SDK_MAXSCENES 100
#define SIMMEDTK_SDK_MAXSCENEOBJTECTS 100

class smMotionTransformer;
template<typename T> class IndiceArray;
class Scene;

/// \brief module registration
enum smSDKReturnType
{
    SIMMEDTK_SDK_MODULEREGISTERED,
    SIMMEDTK_SDK_MODULEREGISTEREDALREADY
};


/// \brief mesh holder
struct MeshHolder
{

    MeshHolder()
    {
        mesh = NULL;
    }

    std::shared_ptr<BaseMesh> mesh;

    inline bool operator ==(MeshHolder &p_param)
    {
        return mesh == p_param.mesh;
    }
};

/// \brief module holder
struct ModuleHolder
{

    ModuleHolder()
    {
        module = NULL;
    }

    std::shared_ptr<Module> module;

    inline bool operator ==(ModuleHolder &p_param)
    {
        return module == p_param.module;
    }
};

/// \brief simulator holder
struct ObjectSimulatorHolder
{

    ObjectSimulatorHolder()
    {
        objectSim = NULL;
    }

    std::shared_ptr<ObjectSimulator> objectSim;

    inline bool operator ==(ObjectSimulatorHolder &p_param)
    {
        return objectSim == p_param.objectSim;
    }
};

/// \brief scene holders
struct SceneHolder
{
    SceneHolder()
    {
        scene = NULL;
    }

    std::shared_ptr<Scene> scene;
    inline bool operator ==(SceneHolder &p_param)
    {
        return scene == p_param.scene;
    }

};

/// \brief scene object holder
struct SceneObjectHolder
{

    SceneObjectHolder()
    {
        sceneObject = NULL;
    }

    std::shared_ptr<SceneObject> sceneObject;
    inline bool operator ==(SceneObjectHolder &p_param)
    {
        return sceneObject == p_param.sceneObject;
    }

};


/// \brief SDK class. it is a singlenton class for each machine runs the framework
class SDK: public CoreClass
{
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
    std::vector<std::shared_ptr<Scene>> sceneList; ///< scene list

    ///holds the references to the entities in the framework
    static IndiceArray<MeshHolder> *meshesRef;
    static IndiceArray<ModuleHolder> *modulesRef;
    static IndiceArray<ObjectSimulatorHolder> *objectSimulatorsRef;
    static IndiceArray<ObjectSimulatorHolder> *collisionDetectorsRef;
    static IndiceArray<SceneHolder> *scenesRef;
    static IndiceArray<SceneObjectHolder> *sceneObjectsRef;

    std::vector<std::thread> modules; ///< Stores a list of running module threads

    /// \brief constructor
    SDK();

public:
    /// \brief destructor
    ~SDK();

    /// \brief update scene list. not implemented
    void updateSceneListAll();

    void initRegisteredModules();
    int runRegisteredModules();
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
        return scenesRef->getByRef(sceneId).scene;
    }

    ///SDK returns logger for the system
    std::shared_ptr<ErrorLog> getErrorLog();

    ///terminates every module. Do it later on with smMessager
    void terminateAll();

    ///release the scene from the SDK..not implemented yet
    void releaseScene(std::shared_ptr<Scene> scene);

    /// \brief run the SDK
    void run();

    /// \brief add reference to a core class
    void addRef(std::shared_ptr<CoreClass> p_coreClass);

    /// \brief removes reference on core class
    void removeRef(std::shared_ptr<CoreClass> p_coreClass);

    /// \brief register functions
    int registerMesh(std::shared_ptr<BaseMesh> p_mesh);

    int registerModule(std::shared_ptr<Module> p_mod);

    void registerObjectSim(std::shared_ptr<ObjectSimulator> p_os);

    void registerCollDet(std::shared_ptr<ObjectSimulator> p_col);

    void registerScene(std::shared_ptr<Scene> p_sc);

    void registerSceneObject(std::shared_ptr<SceneObject> p_sco);

    void addSceneActor(std::shared_ptr<SceneObject> p_sco, std::shared_ptr<ObjectSimulator> p_os, int p_scId=0);
};

#endif
