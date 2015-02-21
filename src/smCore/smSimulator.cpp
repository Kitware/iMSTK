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

#include "smCore/smSimulator.h"

/// \brief starts the tasks with the threads from thread pool
void smSimulator::beginFrame()
{

    frameCounter++;
}

/// \brief waits until the frame ends
void smSimulator::endFrame()
{

}

void smSimulator::startAsychThreads()
{

    smInt asyncThreadNbr = 0;

    for (smInt i = 0; i < simulators.size(); i++)
    {
        if (simulators[i]->execType == SIMMEDTK_SIMEXECUTION_ASYNCMODE)
        {
            asyncThreadNbr++;
        }
    }

    asyncPool->setMaxThreadCount(asyncThreadNbr);

    for (smInt i = 0; i < simulators.size(); i++)
    {
        if (simulators[i]->execType == SIMMEDTK_SIMEXECUTION_ASYNCMODE)
        {
            asyncPool->start(simulators[i], simulators[i]->getPriority());
        }
    }
}

/// \brief the main simulation loop
void smSimulator::run()
{

    smObjectSimulator *objectSimulator;
    smInt nbrSims;

    if (isInitialized == false)
    {
        log->addError(this, "Simulator is not initialized.");
        return;
    }

    smSimulationMainParam param;
    param.sceneList = sceneList;

    startAsychThreads();
    simulatorThreadPool->size_controller().resize(this->simulators.size());

    while (true && this->terminateExecution == false)
    {
        beginModule();

        if (main != NULL)
        {
            main->simulateMain(param);
        }

        if (changedMainTimeStamp > mainTimeStamp)
        {

            main = changedMain;
            changedMainTimeStamp = mainTimeStamp;

        }

        nbrSims = simulators.size();
        smTimer timer;

        for (smInt i = 0; i < this->simulators.size(); i++)
        {
            objectSimulator = simulators[i];

            if (objectSimulator->execType == SIMMEDTK_SIMEXECUTION_ASYNCMODE)
            {
                continue;
            }

            if (objectSimulator->enabled == false)
            {
                continue;
            }

            schedule(*simulatorThreadPool, boost::bind(&smObjectSimulator::run, objectSimulator));
        }

        simulatorThreadPool->wait();

        for (smInt i = 0; i < this->simulators.size(); i++)
        {
            objectSimulator = simulators[i];
            objectSimulator->syncBuffers();
        }

        timer.start();

        for (smInt i = 0; i < this->collisionDetectors.size(); i++)
        {
            objectSimulator = collisionDetectors[i];
            schedule(*simulatorThreadPool, boost::bind(&smObjectSimulator::run, objectSimulator));
        }

        simulatorThreadPool->wait();

        endModule();
    }

    asyncPool->waitForDone();

}

/// \brief
void smSimulator::registerObjectSimulator(smObjectSimulator *objectSimulator)
{

    simulators.push_back(objectSimulator);
    objectSimulator->enabled = true;
}

/// \brief
void smSimulator::registerCollisionDetection(smObjectSimulator *p_collisionDetection)
{

    collisionDetectors.push_back(p_collisionDetection);
}

/// \brief
void smSimulator::registerSimulationMain(smSimulationMain*p_main)
{

    changedMain = p_main;
    this->changedMainTimeStamp++;
}
