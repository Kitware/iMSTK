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

#include "smCore/smSDK.h"
#include "smMesh/smMesh.h"

#include <chrono>
#include <thread>
#include <string>

/// \brief SDK is singlenton class
// std::unique_ptr<smErrorLog> smSDK::errorLog;
std::shared_ptr<smSDK> smSDK::sdk;
std::once_flag smSDK::sdkCallOnceFlag;

smIndiceArray<smMeshHolder>  *smSDK::meshesRef;
smIndiceArray<smModuleHolder> *smSDK::modulesRef;
smIndiceArray<smObjectSimulatorHolder>  *smSDK::objectSimulatorsRef;
smIndiceArray<smObjectSimulatorHolder>*smSDK::collisionDetectorsRef;
smIndiceArray<smSceneHolder>*smSDK::scenesRef;
smIndiceArray<smSceneObjectHolder>*smSDK::sceneObjectsRef;
smIndiceArray<smMotionTransformer*> *smSDK::motionTransRef;
smIndiceArray<smPipeHolder> *smSDK::pipesRef;


/// \brief creates the scene of the simulator
std::shared_ptr<smScene> smSDK::createScene()
{
    auto scene = std::make_shared<smScene>(errorLog);
    registerScene(scene);
    scene->setName("Scene" + std::to_string(scene->uniqueId.ID));
    return scene;
}

void smSDK::releaseScene(std::shared_ptr<smScene> scene)
{
    unRegisterScene(scene);
    scene.reset();
}

smSDK::~smSDK()
{
    std::cout << "Killing SDK" << std::endl;
}

void smSDK::addViewer(std::shared_ptr<smViewer> p_viewer)
{
    assert(p_viewer);

    this->viewer = p_viewer;
    this->viewer->log = this->errorLog;
    this->viewer->dispathcer = this->dispathcer;

    this->registerModule(p_viewer);
}

/// \brief Returns a pointer to the viewer object
///
/// \return Returns a pointer to the viewer object
std::shared_ptr<smViewer> smSDK::getViewerInstance()
{
    return this->viewer;
}

/// \brief
std::shared_ptr<smSimulator> smSDK::createSimulator()
{
    if (this->simulator == nullptr)
    {
        simulator = std::make_shared<smSimulator>(errorLog);
        simulator->dispathcer = dispathcer;

        for (smInt j = 0; j < (*scenesRef).size(); j++)
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

    for (smInt i = 0; i < modulesRef->size(); i++)
        if ((*modulesRef)[i].module->getType() != SIMMEDTK_SMVIEWER)
        {
            (*modulesRef)[i].module->init();
        }
}

/// \brief
void smSDK::runRegisteredModules()
{

    if (isModulesStarted)
    {
        return;
    }

    for (smInt i = 0; i < modulesRef->size(); i++)
    {
        modules.emplace_back([i]{(*modulesRef)[i].module->exec();});
    }

    isModulesStarted = true;
}

/// \brief
void smSDK::shutDown()
{

    for (smInt i = 0; i < modulesRef->size(); i++)
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

    runRegisteredModules();
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
    std::call_once(sdkCallOnceFlag,
                   []
                    {
                        sdk.reset(new smSDK);
                    });
    return sdk;
}

std::shared_ptr<smSDK> smSDK::getInstance()
{
    return smSDK::createSDK();
}
void smSDK::terminateAll()
{

    for(smInt i = 0; i < (*modulesRef).size(); i++)
    {
        (*modulesRef)[i].module->terminateExecution = true;
    }

    for(smInt i = 0; i < (*modulesRef).size(); i++)
    {
        smBool moduleTerminated = false;

        while(true && !moduleTerminated)
        {
            if((*modulesRef)[i].module->isTerminationDone())
            {
                moduleTerminated = true;
            }
        }
    }
}
