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
#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smCore/smScene.h"
#include "smRendering/smViewer.h"
#include "smCore/smSimulator.h"
#include "smCore/smTextureManager.h"
#include "smCore/smModule.h"
#include "smUtilities/smDataStructures.h"
#include "smUtilities/smMakeUnique.h"

/// \brief maximum entities in the framework
#define SIMMEDTK_SDK_MAXMESHES 100
#define SIMMEDTK_SDK_MAXMODULES 100
#define SIMMEDTK_SDK_MAXOBJECTSIMULATORS 100
#define SIMMEDTK_SDK_MAXSCENES 100
#define SIMMEDTK_SDK_MAXSCENEOBJTECTS 100

class smMotionTransformer;
class smPipe;
template<typename T> class smIndiceArray;
class smScene;
/// \brief module registration
enum smSDKReturnType
{
    SIMMEDTK_SDK_MODULEREGISTERED,
    SIMMEDTK_SDK_MODULEREGISTEREDALREADY
};

struct smBaseHolder
{
};
/// \brief mesh holder
struct smMeshHolder: public smBaseHolder
{

    smMeshHolder()
    {
        mesh = NULL;
    }

    std::shared_ptr<smBaseMesh> mesh;

    inline smBool operator ==(smMeshHolder &p_param)
    {
        return mesh == p_param.mesh;
    }
};
/// \brief module holder
struct smModuleHolder: public smBaseHolder
{

    smModuleHolder()
    {
        module = NULL;
    }

    std::shared_ptr<smModule> module;

    inline smBool operator ==(smModuleHolder &p_param)
    {
        return module == p_param.module;
    }
};
/// \brief simulator holder
struct smObjectSimulatorHolder: public smBaseHolder
{

    smObjectSimulatorHolder()
    {
        objectSim = NULL;
    }

    std::shared_ptr<smObjectSimulator> objectSim;

    inline smBool operator ==(smObjectSimulatorHolder &p_param)
    {
        return objectSim == p_param.objectSim;
    }
};
/// \brief scene holders
struct smSceneHolder: public smBaseHolder
{
    smSceneHolder()
    {
        scene = NULL;
    }

    std::shared_ptr<smScene> scene;
    inline smBool operator ==(smSceneHolder &p_param)
    {
        return scene == p_param.scene;
    }

};
/// \brief scene object holder
struct smSceneObjectHolder: public smBaseHolder
{

    smSceneObjectHolder()
    {
        sceneObject = NULL;
    }

    std::shared_ptr<smSceneObject> sceneObject;
    inline smBool operator ==(smSceneObjectHolder &p_param)
    {
        return sceneObject == p_param.sceneObject;
    }

};
/// \brief pipe holder
struct smPipeHolder: public smBaseHolder
{

    smPipe *pipe;

    smPipeHolder()
    {
        pipe = NULL;
    }

    inline smBool operator ==(smPipeHolder &p_param)
    {
        return pipe == p_param.pipe;
    }

    inline friend smBool operator==(smPipeHolder &p_pipe, smString &p_name)
    {
        return (*(p_pipe.pipe) == p_name);
    }

};
/// \brief SDK class. it is a singlenton class for each machine runs the framework
class smSDK: public smCoreClass
{
private:
    static std::shared_ptr<smSDK> sdk; ///< singleton sdk.
    static std::once_flag sdkCallOnceFlag;

    bool shutdown; ///< Tells the SDK to terminate
    smInt sceneIdCounter; ///< this id is incremented when a scene is created
    smInt argc;
    smChar argv;
    smBool isModulesStarted;

    std::shared_ptr<smErrorLog> errorLog; ///< error log
    std::shared_ptr<smViewer> viewer; ///< Reference to the sdk viewer object
    std::shared_ptr<smSimulator> simulator; ///< Reference to the sdk simulator object
    std::vector<std::shared_ptr<smScene>> sceneList; ///< scene list

    ///holds the references to the entities in the framework
    static smIndiceArray<smMeshHolder> *meshesRef;
    static smIndiceArray<smModuleHolder> *modulesRef;
    static smIndiceArray<smObjectSimulatorHolder> *objectSimulatorsRef;
    static smIndiceArray<smObjectSimulatorHolder> *collisionDetectorsRef;
    static smIndiceArray<smSceneHolder> *scenesRef;
    static smIndiceArray<smSceneObjectHolder> *sceneObjectsRef;
    static smIndiceArray<smMotionTransformer *> *motionTransRef;
    static smIndiceArray<smPipeHolder>* pipesRef;

    std::vector<std::thread> modules; ///< Stores a list of running module threads

    /// \brief constructor
    smSDK()
    {
        shutdown = false;
        sceneIdCounter = 1;
        isModulesStarted = false;
        type = SIMMEDTK_SMSDK;
        viewer = nullptr;
        simulator = nullptr;
        sceneList.clear();

        errorLog = std::make_shared<smErrorLog>();

        // TODO: Fix these! Leaking...
        meshesRef = new smIndiceArray<smMeshHolder>(SIMMEDTK_SDK_MAXMESHES);
        modulesRef = new smIndiceArray<smModuleHolder>(SIMMEDTK_SDK_MAXMODULES) ;
        objectSimulatorsRef = new smIndiceArray<smObjectSimulatorHolder>(SIMMEDTK_SDK_MAXOBJECTSIMULATORS);
        collisionDetectorsRef = new smIndiceArray<smObjectSimulatorHolder>(SIMMEDTK_SDK_MAXOBJECTSIMULATORS) ;
        scenesRef = new smIndiceArray<smSceneHolder>(SIMMEDTK_SDK_MAXSCENES);
        sceneObjectsRef = new smIndiceArray<smSceneObjectHolder>(SIMMEDTK_SDK_MAXSCENEOBJTECTS);
        pipesRef = new smIndiceArray<smPipeHolder>(SIMMEDTK_SDK_MAXSCENEOBJTECTS);
    }

public:
    /// \brief destructor
    ~smSDK();

    /// \brief update scene list. not implemented
    void updateSceneListAll();

    /// \brief init registered modules
    void initRegisteredModules();

    /// \brief run the registered modules
    void runRegisteredModules();

    ///shutdowns all the modules
    void shutDown();

    ///for now both functions below are the same. But it maybe subject to change.
    static std::shared_ptr<smSDK> createSDK();

    static std::shared_ptr<smSDK> getInstance();

    /// \brief Registers a viewer object with the SDK
    ///
    void addViewer(std::shared_ptr<smViewer> p_viewer);

    ///SDK returns a pointer to the viewer
    std::shared_ptr<smViewer> getViewerInstance();

    ///SDK creates simualtor
    std::shared_ptr<smSimulator> createSimulator();

    ///SDK creates simualtor
    std::shared_ptr<smSimulator> getSimulator()
    {
        return this->simulator;
    }

    ///SDK creates scene
    std::shared_ptr<smScene> createScene();

    ///SDK returns logger for the system
    std::shared_ptr<smErrorLog> getErrorLog()
    {
        return errorLog;
    };

    ///terminates every module. Do it later on with smMessager
    void terminateAll();

    ///release the scene from the SDK..not implemented yet
    void releaseScene(std::shared_ptr<smScene> scene);

    /// \brief run the SDK
    void run();

    /// \brief add reference to a core class
    void addRef(std::shared_ptr<smCoreClass> p_coreClass);

    /// \brief removes reference on core class
    void removeRef(std::shared_ptr<smCoreClass> p_coreClass);

    /// \brief register functions
    smInt registerMesh(std::shared_ptr<smBaseMesh> p_mesh)
    {
        smMeshHolder mh;
        mh.mesh = p_mesh;
        return meshesRef->checkAndAdd(mh);
    }

    smInt registerModule(std::shared_ptr<smModule> p_mod)
    {
        smModuleHolder mh;
        mh.module = p_mod;
        return modulesRef->checkAndAdd(mh);
    }

    void registerObjectSim(std::shared_ptr<smObjectSimulator> p_os)
    {
        smObjectSimulatorHolder os;
        os.objectSim = p_os;
        objectSimulatorsRef->checkAndAdd(os);
    }

    void registerCollDet(std::shared_ptr<smObjectSimulator> p_col)
    {
        smObjectSimulatorHolder col;
        col.objectSim = p_col;
        collisionDetectorsRef->checkAndAdd(col);
    }

    void registerScene(std::shared_ptr<smScene> p_sc)
    {
        smSceneHolder sc;
        sc.scene = p_sc;
        scenesRef->checkAndAdd(sc);
    }

    void registerSceneObject(std::shared_ptr<smSceneObject> p_sco)
    {
        smSceneObjectHolder  sh;
        sh.sceneObject = p_sco;
        sceneObjectsRef->checkAndAdd(sh);
    }

    /// \brief getter functions

    smPipe* getPipeByName(smString p_name)
    {
        return (pipesRef->getByRef(p_name).pipe);
    }

    /// \brief register pipe
    void registerPipe(smPipe*p_pipe)
    {
        smPipeHolder ph;
        ph.pipe = p_pipe;
        pipesRef->checkAndAdd(ph);
    }

    /// \brief create a pipe
    smPipe *createPipe(smString p_pipeName, smInt p_elementSize, smInt p_size)
    {
        smPipe *pipe;
        pipe = new smPipe(p_pipeName, p_elementSize, p_size);
        registerPipe(pipe);
        return pipe;
    }
};

#endif
