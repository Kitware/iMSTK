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

#include "SDK.h"
#include "Factory.h"

#include <chrono>
#include <thread>
#include <string>

/// \brief SDK is singlenton class
// std::unique_ptr<ErrorLog> SDK::errorLog;
std::once_flag SDK::sdkCallOnceFlag;

IndiceArray<MeshHolder>  *SDK::meshesRef;
IndiceArray<ModuleHolder> *SDK::modulesRef;
IndiceArray<ObjectSimulatorHolder>  *SDK::objectSimulatorsRef;
IndiceArray<ObjectSimulatorHolder>*SDK::collisionDetectorsRef;
IndiceArray<SceneHolder>*SDK::scenesRef;
IndiceArray<SceneObjectHolder>*SDK::sceneObjectsRef;

/// \brief constructor
SDK::SDK()
{
    shutdown = false;
    sceneIdCounter = 1;
    isModulesStarted = false;
    type = core::ClassType::Sdk;
    viewer = nullptr;
    simulator = nullptr;
    sceneList.clear();

    errorLog = std::make_shared<ErrorLog>();

    // TODO: Fix these! Leaking...
    meshesRef = new IndiceArray<MeshHolder>(SIMMEDTK_SDK_MAXMESHES);
    modulesRef = new IndiceArray<ModuleHolder>(SIMMEDTK_SDK_MAXMODULES) ;
    objectSimulatorsRef = new IndiceArray<ObjectSimulatorHolder>(SIMMEDTK_SDK_MAXOBJECTSIMULATORS);
    collisionDetectorsRef = new IndiceArray<ObjectSimulatorHolder>(SIMMEDTK_SDK_MAXOBJECTSIMULATORS) ;
    scenesRef = new IndiceArray<SceneHolder>(SIMMEDTK_SDK_MAXSCENES);
    sceneObjectsRef = new IndiceArray<SceneObjectHolder>(SIMMEDTK_SDK_MAXSCENEOBJTECTS);
}

SDK::~SDK()
{
    std::cout << "Killing SDK" << std::endl;
}

/// \brief creates the scene of the simulator
std::shared_ptr<Scene> SDK::createScene()
{
    auto scene = std::make_shared<Scene>(errorLog);
    registerScene(scene);
    scene->setName("Scene" + std::to_string(scene->getUniqueId()->getId()));
    return scene;
}

void SDK::releaseScene(std::shared_ptr<Scene> scene)
{
    scene.reset();
}

std::shared_ptr<ViewerBase> SDK::createViewer()
{
    this->viewer = Factory<CoreClass>::createDefaultAs<ViewerBase>("ViewerBase");
    if (!!this->viewer)
      {
      this->viewer->log = this->errorLog;
      this->registerModule(this->viewer);
      }

    return this->viewer;
}

void SDK::addViewer(std::shared_ptr<ViewerBase> p_viewer)
{
    assert(p_viewer);

    this->viewer = p_viewer;
    this->viewer->log = this->errorLog;
    this->registerModule(p_viewer);
}

/// \brief Returns a pointer to the viewer object
///
/// \return Returns a pointer to the viewer object
std::shared_ptr<ViewerBase> SDK::getViewerInstance()
{
    return this->viewer;
}

/// \brief
std::shared_ptr<Simulator> SDK::createSimulator()
{
    if (this->simulator == nullptr)
    {
        simulator = std::make_shared<Simulator>(errorLog);

        for (int j = 0; j < (*scenesRef).size(); j++)
        {
            simulator->sceneList.push_back((*scenesRef)[j].scene);
        }

        registerModule(simulator);
    }

    return simulator;
}

/// \brief
void SDK::updateSceneListAll()
{
}

/// \brief Initialize all modules registered to the SimMedTK SDK
void SDK::initRegisteredModules()
{

    for (int i = 0; i < modulesRef->size(); i++)
        if ((*modulesRef)[i].module->getType() != core::ClassType::Viewer)
        {
            (*modulesRef)[i].module->init();
        }
}

/** \brief Run the registered modules
  *
  * This will not run any modules that inherit ViewerBase as
  * on some platforms (Mac OS X) only the main thread can run
  * user interface code. The return value is -1 if the modules
  * are already running or no module inherits ViewerBase.
  * Otherwise, the index of the last viewer module encountered
  * is returned.
  */
int SDK::runRegisteredModules()
{
    int viewerIndex = -1;

    if (isModulesStarted)
    {
        return viewerIndex;
    }

    for (int i = 0; i < modulesRef->size(); i++)
    {
        auto view = std::dynamic_pointer_cast<ViewerBase>((*modulesRef)[i].module);
        if (view)
          viewerIndex = i;
        else
          modules.emplace_back([i]{(*modulesRef)[i].module->exec();});
    }

    isModulesStarted = true;
    return viewerIndex;
}

///\brief shutdowns all the modules
void SDK::shutDown()
{

    for (int i = 0; i < modulesRef->size(); i++)
    {
        (*modulesRef)[i].module->terminateExecution = true;
    }
    shutdown = true;
}

/// \brief runs the simulator
void SDK::run()
{
    updateSceneListAll();
    initRegisteredModules();

    int viewer = runRegisteredModules();
    // Run the viewer in the main thread:
    if (viewer >= 0)
        (*modulesRef)[viewer].module->exec();
    // Now wait for other modules to shut down
    while (!shutdown) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    terminateAll(); //Tell framework threads to shutdown
    //Wait for all threads to finish processing
    for (size_t i = 0; i < modules.size(); ++i)
    {
        modules[i].join();
    }
}

/// \brief
void SDK::addRef(std::shared_ptr<CoreClass> p_coreClass)
{
    ++*p_coreClass;
}

/// \brief
void SDK::removeRef(std::shared_ptr<CoreClass> p_coreClass)
{
    --*p_coreClass;
}

std::shared_ptr<SDK> SDK::createSDK()
{
    static std::shared_ptr<SDK> sdk; ///< singleton sdk.
    std::call_once(sdkCallOnceFlag,
                   []
                    {
                        sdk.reset(new SDK);
                    });
    return sdk;
}

std::shared_ptr<SDK> SDK::createStandardSDK()
{
    auto sdk = createSDK();

    sdk->createScene();
    sdk->createViewer();

    sdk->createSimulator();

    return sdk;
}

std::shared_ptr<SDK> SDK::getInstance()
{
    return SDK::createSDK();
}

void SDK::terminateAll()
{

    for(int i = 0; i < (*modulesRef).size(); i++)
    {
        (*modulesRef)[i].module->terminateExecution = true;
    }

    for(int i = 0; i < (*modulesRef).size(); i++)
    {
        bool moduleTerminated = false;

        while(true && !moduleTerminated)
        {
            if((*modulesRef)[i].module->isTerminationDone())
            {
                moduleTerminated = true;
            }
        }
    }
}

/// \brief register functions
int SDK::registerMesh(std::shared_ptr<BaseMesh> p_mesh)
{
    MeshHolder mh;
    mh.mesh = p_mesh;
    return meshesRef->checkAndAdd(mh);
}

int SDK::registerModule(std::shared_ptr<Module> p_mod)
{
    ModuleHolder mh;
    mh.module = p_mod;
    return modulesRef->checkAndAdd(mh);
}

void SDK::registerObjectSim(std::shared_ptr<ObjectSimulator> p_os)
{
    ObjectSimulatorHolder os;
    os.objectSim = p_os;
    objectSimulatorsRef->checkAndAdd(os);
}

void SDK::registerCollDet(std::shared_ptr<ObjectSimulator> p_col)
{
    ObjectSimulatorHolder col;
    col.objectSim = p_col;
    collisionDetectorsRef->checkAndAdd(col);
}

void SDK::registerScene(std::shared_ptr<Scene> p_sc)
{
    SceneHolder sc;
    sc.scene = p_sc;
    scenesRef->checkAndAdd(sc);
}

void SDK::registerSceneObject(std::shared_ptr<SceneObject> p_sco)
{
    SceneObjectHolder  sh;
    sh.sceneObject = p_sco;
    sceneObjectsRef->checkAndAdd(sh);
}

void SDK::addSceneActor(std::shared_ptr<SceneObject> p_sco, std::shared_ptr<ObjectSimulator> p_os, int p_scId)
{
    assert(p_os);
    assert(p_sco);

    p_sco->attachObjectSimulator(p_os);

    this->registerObjectSim(p_os);

    this->registerSceneObject(p_sco);

    this->getScene(p_scId)->addSceneObject(p_sco);
}

///SDK returns logger for the system
std::shared_ptr<ErrorLog> SDK::getErrorLog()
{
    return errorLog;
};

std::shared_ptr<Simulator> SDK::getSimulator()
{
    return this->simulator;
}
