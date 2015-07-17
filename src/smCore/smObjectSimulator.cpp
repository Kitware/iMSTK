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

#include "smObjectSimulator.h"
#include "smRendering/smViewer.h"
#include "smSDK.h"

smObjectSimulator::smObjectSimulator(std::shared_ptr<smErrorLog> p_log)
{
    this->log = p_log;
//     smSDK::getInstance()->registerObjectSim(safeDownCast<smObjectSimulator>());
    name = "objecSimulator" + std::to_string(this->getUniqueId()->getId());

    type = SIMMEDTK_SMOBJECTSIMULATOR;
    isObjectSimInitialized = false;
    threadPriority = SIMMEDTK_THREAD_NORMALPRIORITY;

    objectsSimulated.clear();
    FPS = 0.0;
    frameCounter = 0;
    totalTime = 0.0;
    timer.start();
    enabled = false;
    executionTypeStatusChanged = false;
    execType = SIMMEDTK_SIMEXECUTION_SYNCMODE;
}

void smObjectSimulator::addObject(std::shared_ptr<smSceneObject> p_object)
{
    p_object->objectSim = safeDownCast<smObjectSimulator>();
    objectsSimulated.emplace_back( p_object );
}

void smObjectSimulator::removeObject(std::shared_ptr<smSceneObject> /*p_object*/ )
{
}

void smObjectSimulator::setPriority( smThreadPriority p_priority )
{
    threadPriority = p_priority;
}

void smObjectSimulator::setExecutionType( smSimulatorExecutionType p_type )
{
    if ( execType != p_type )
    {
        executionTypeStatusChanged = true;
    }

    execType = p_type;
}

smThreadPriority smObjectSimulator::getPriority()
{
    return threadPriority;
}

void smObjectSimulator::init()
{
    if ( isObjectSimInitialized == false )
    {
        initCustom();
        //make the simulator true..it is initialized
        isObjectSimInitialized = true;
    }
}

void smObjectSimulator::beginSim()
{
    frameCounter++;
    timer.start();
}
void smObjectSimulator::endSim()
{
    timerPerFrame = timer.elapsed();
    totalTime += timerPerFrame;

    if ( totalTime > 1.0 )
    {
        FPS = frameCounter;
        frameCounter = 0.0;
        totalTime = 0.0;
    }
}

void smObjectSimulator::updateSceneList()
{
}

smObjectSimulator::smObjectSimulatorObjectIter::smObjectSimulatorObjectIter( smScheduleGroup &p_group,
                                                                             std::vector<std::shared_ptr<smSceneObject>> &p_objectsSimulated,
                                                                             int p_threadIndex )
{

    smInt objectsPerThread;
    smInt leap;
    threadIndex = p_threadIndex;
    smInt totalObjects = p_objectsSimulated.size();
    leap = ( totalObjects % p_group.totalThreads );
    objectsPerThread = p_objectsSimulated.size() / ( p_group.totalThreads );

    if ( threadIndex == 0 )
    {
        beginIndex = 0;
        endIndex = objectsPerThread + ( leap != 0 ? 1 : 0 );

    }
    else
    {
        beginIndex = objectsPerThread * threadIndex;

        if ( threadIndex < leap && leap != 0 )
        {
            beginIndex += threadIndex;
        }
        else
        {
            beginIndex += leap;
        }

        endIndex = beginIndex + objectsPerThread;

        if ( endIndex < leap && leap != 0 )
        {
            endIndex++;
        }
    }
}
void smObjectSimulator::smObjectSimulatorObjectIter::setThreadIndex( short int p_threadIndex )
{
    threadIndex = p_threadIndex;
}

int smObjectSimulator::smObjectSimulatorObjectIter::begin()
{
    return beginIndex;
}

int smObjectSimulator::smObjectSimulatorObjectIter::end()
{
    return endIndex;
}
