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
// std::unique_ptr<smErrorLog> smSDK::errorLog;
std::once_flag smSDK::sdkCallOnceFlag;

smIndiceArray<smMeshHolder>  *smSDK::meshesRef;
smIndiceArray<smModuleHolder> *smSDK::modulesRef;
smIndiceArray<smObjectSimulatorHolder>  *smSDK::objectSimulatorsRef;
smIndiceArray<smObjectSimulatorHolder>*smSDK::collisionDetectorsRef;
smIndiceArray<smSceneHolder>*smSDK::scenesRef;
smIndiceArray<smSceneObjectHolder>*smSDK::sceneObjectsRef;

/// \brief constructor
smSDK::smSDK()
{
    shutdown = false;
    sceneIdCounter = 1;
    isModulesStarted = false;
    type = core::ClassType::Sdk;
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
}

smSDK::~smSDK()
{
    std::cout << "Killing SDK" << std::endl;
}

/// \brief creates the scene of the simulator
std::shared_ptr<smScene> smSDK::createScene()
{
    auto scene = std::make_shared<smScene>(errorLog);
    registerScene(scene);
    scene->setName("Scene" + std::to_string(scene->getUniqueId()->getId()));
    return scene;
}

void smSDK::releaseScene(std::shared_ptr<smScene> scene)
{
    scene.reset();
}

std::shared_ptr<smViewerBase> smSDK::createViewer()
{
    this->viewer = smFactory<smCoreClass>::createDefaultAs<smViewerBase>("ViewerBase");
    if (!!this->viewer)
      {
      this->viewer->log = this->errorLog;
      this->registerModule(this->viewer);
      }

    return this->viewer;
}

void smSDK::addViewer(std::shared_ptr<smViewerBase> p_viewer)
{
    assert(p_viewer);

    this->viewer = p_viewer;
    this->viewer->log = this->errorLog;
    this->registerModule(p_viewer);
}

/// \brief Returns a pointer to the viewer object
///
/// \return Returns a pointer to the viewer object
std::shared_ptr<smViewerBase> smSDK::getViewerInstance()
{
    return this->viewer;
}

/// \brief
std::shared_ptr<smSimulator> smSDK::createSimulator()
{
    if (this->simulator == nullptr)
    {
        simulator = std::make_shared<smSimulator>(errorLog);

        for (int j = 0; j < (*scenesRef).size(); j++)
        {
            simulator->sceneList.push_back((*scenesRef)[j].scene);
        }

        registerModule(simulator);
    }

    return simulator;
}

/// \brief
void smSDK::updateSceneListAll()
{
}

/// \brief Initialize all modules registered to the SimMedTK SDK
void smSDK::initRegisteredModules()
{

    for (int i = 0; i < modulesRef->size(); i++)
        if ((*modulesRef)[i].module->getType() != core::ClassType::Viewer)
        {
            (*modulesRef)[i].module->init();
        }
}

/** \brief Run the registered modules
  *
  * This will not run any modules that inherit smViewerBase as
  * on some platforms (Mac OS X) only the main thread can run
  * user interface code. The return value is -1 if the modules
  * are already running or no module inherits smViewerBase.
  * Otherwise, the index of the last viewer module encountered
  * is returned.
  */
int smSDK::runRegisteredModules()
{
    int viewerIndex = -1;

    if (isModulesStarted)
    {
        return viewerIndex;
    }

    for (int i = 0; i < modulesRef->size(); i++)
    {
        auto view = std::dynamic_pointer_cast<smViewerBase>((*modulesRef)[i].module);
        if (view)
          viewerIndex = i;
        else
          modules.emplace_back([i]{(*modulesRef)[i].module->exec();});
    }

    isModulesStarted = true;
    return viewerIndex;
}

///\brief shutdowns all the modules
void smSDK::shutDown()
{

    for (int i = 0; i < modulesRef->size(); i++)
    {
        (*modulesRef)[i].module->terminateExecution = true;
    }
    shutdown = true;
}

/// \brief runs the simulator
void smSDK::run()
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
void smSDK::addRef(std::shared_ptr<smCoreClass> p_coreClass)
{
    ++*p_coreClass;
}

/// \brief
void smSDK::removeRef(std::shared_ptr<smCoreClass> p_coreClass)
{
    --*p_coreClass;
}

std::shared_ptr<smSDK> smSDK::createSDK()
{
    static std::shared_ptr<smSDK> sdk; ///< singleton sdk.
    std::call_once(sdkCallOnceFlag,
                   []
                    {
                        sdk.reset(new smSDK);
                    });
    return sdk;
}

std::shared_ptr<smSDK> smSDK::createStandardSDK()
{
    auto sdk = createSDK();

    sdk->createScene();
    sdk->createViewer();

    sdk->createSimulator();

    return sdk;
}

std::shared_ptr<smSDK> smSDK::getInstance()
{
    return smSDK::createSDK();
}

void smSDK::terminateAll()
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
int smSDK::registerMesh(std::shared_ptr<smBaseMesh> p_mesh)
{
    smMeshHolder mh;
    mh.mesh = p_mesh;
    return meshesRef->checkAndAdd(mh);
}

int smSDK::registerModule(std::shared_ptr<smModule> p_mod)
{
    smModuleHolder mh;
    mh.module = p_mod;
    return modulesRef->checkAndAdd(mh);
}

void smSDK::registerObjectSim(std::shared_ptr<smObjectSimulator> p_os)
{
    smObjectSimulatorHolder os;
    os.objectSim = p_os;
    objectSimulatorsRef->checkAndAdd(os);
}

void smSDK::registerCollDet(std::shared_ptr<smObjectSimulator> p_col)
{
    smObjectSimulatorHolder col;
    col.objectSim = p_col;
    collisionDetectorsRef->checkAndAdd(col);
}

void smSDK::registerScene(std::shared_ptr<smScene> p_sc)
{
    smSceneHolder sc;
    sc.scene = p_sc;
    scenesRef->checkAndAdd(sc);
}

void smSDK::registerSceneObject(std::shared_ptr<smSceneObject> p_sco)
{
    smSceneObjectHolder  sh;
    sh.sceneObject = p_sco;
    sceneObjectsRef->checkAndAdd(sh);
}

void smSDK::addSceneActor(std::shared_ptr<smSceneObject> p_sco, std::shared_ptr<smObjectSimulator> p_os, int p_scId)
{
    assert(p_os);
    assert(p_sco);

    p_sco->attachObjectSimulator(p_os);

    this->registerObjectSim(p_os);

    this->registerSceneObject(p_sco);

    this->getScene(p_scId)->addSceneObject(p_sco);
}

///SDK returns logger for the system
std::shared_ptr<smErrorLog> smSDK::getErrorLog()
{
    return errorLog;
};

std::shared_ptr<smSimulator> smSDK::getSimulator()
{
    return this->simulator;
}
