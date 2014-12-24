/*=========================================================================
 * Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
 *                        Rensselaer Polytechnic Institute
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 /=========================================================================
 
 /**
  *  \brief
  *  \details
  *  \author
  *  \author
  *  \copyright Apache License, Version 2.0.
  */

#include <QMutex>

#include "smCore/smSDK.h"
#include "smMesh/smMesh.h"

/// \brief SDK is singlenton class
smSDK smSDK::sdk;
smErrorLog * smSDK::errorLog;

/// \brief object reference counter mutex
QMutex  objectRefMutex;

/// \brief this mutex is for system global registration
QMutex  globalRegisterMutex;
QHash<smInt, smSceneObject*> sceneObjectList;

smIndiceArray<smMeshHolder>  *smSDK::meshesRef;
smIndiceArray<smModuleHolder> *smSDK::modulesRef;
smIndiceArray<smObjectSimulatorHolder>  *smSDK::objectSimulatorsRef;
smIndiceArray<smObjectSimulatorHolder>*smSDK::collisionDetectorsRef;
smIndiceArray<smSceneHolder>*smSDK::scenesRef;
smIndiceArray<smSceneObjectHolder>*smSDK::sceneObjectsRef;
smIndiceArray<smMotionTransformer *> *smSDK::motionTransRef;
smIndiceArray<smPipeHolder> *smSDK::pipesRef;

/// \brief creates the scene of the simulator
smScene  *smSDK::createScene()
{

    smScene*scene;
    scene = new smScene(errorLog);
    registerScene(scene);
    scene->setName(QString("Scene") + QString().setNum(scene->uniqueId.ID));
    return scene;
}

smSDK::~smSDK()
{

}

/// \brief creates the viewer for the simulator
smViewer *smSDK::createViewer()
{

    if (this->viewer == NULL)
    {

        viewer = new smViewer(errorLog);
        viewer->dispathcer = dispathcer;

        for (smInt j = 0; j < (*scenesRef).size(); j++)
        {
            viewer->sceneList.push_back((*scenesRef)[j].scene);
        }

        registerModule(viewer);
    }

    return viewer;
}

/// \brief
smSimulator* smSDK::createSimulator()
{

    if (this->simulator == NULL)
    {
        simulator = new smSimulator(errorLog);
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

    QHash<smInt, smModule*>::iterator moduleIterator;
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
        (*modulesRef)[i].module->exec();
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
}

/// \brief runs the simulator
void smSDK::run()
{

    updateSceneListAll();
    initRegisteredModules();

    //previous code where explicit code written for all modules
    if (viewer == NULL)
    {
        errorLog->addError(this, "smSDK:Viewer is not created");
    }
    else
    {
        viewer->exec();
    }

    if (simulator != NULL)
    {
        simulator->exec();
    }

    runRegisteredModules();
    application->exec();
    terminateAll();
}

/// \brief
void smSDK::addRef(smCoreClass* p_coreClass)
{
    objectRefMutex.lock();
    p_coreClass->referenceCounter++;
    objectRefMutex.unlock();
}

/// \brief
void smSDK::removeRef(smCoreClass* p_coreClass)
{
    objectRefMutex.lock();
    p_coreClass->referenceCounter--;
    objectRefMutex.unlock();
}

/// \brief
void smSDK::handleEvent(smEvent *p_event)
{

    smKeyboardEventData *keyBoardData;

    switch (p_event->eventType.eventTypeCode)
    {

    case SIMMEDTK_EVENTTYPE_KEYBOARD:
        keyBoardData = (smKeyboardEventData*)p_event->data;

        if (keyBoardData->keyBoardKey == Qt::Key_Escape)
        {
            terminateAll();
        }

        break;
    }
}
