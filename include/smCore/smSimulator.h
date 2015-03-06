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

#ifndef SMSIMULATOR_H
#define SMSIMULATOR_H
#include "smCore/smModule.h"
#include "smCore/smObjectSimulator.h"

#include <ThreadPool.h>

struct smSimulationMainParam
{
    std::vector<smScene*>sceneList;
};
/// \brief call back for simulator module. simulateMain is called in every simulation module frame.
class smSimulationMain
{

public:
    virtual void simulateMain(smSimulationMainParam) = 0;
};

class smSimulator: public smModule
{
    friend class smSDK;

protected:
    std::vector<smObjectSimulator*> simulators;
    std::vector<smObjectSimulator*> collisionDetectors;

    std::unique_ptr<ThreadPool> threadPool;
    /// \brief asynchronous thread pool
    std::unique_ptr<ThreadPool> asyncPool;
    /// \brief  maximum number of threads
    smInt maxThreadCount;
    /// \brief  error log
    smErrorLog *log;
    /// \brief  module keeps track of frame number
    smUInt frameCounter;

    ///Simulation main registration
    smSimulationMain *main;
    /// \brief  for updating the main in real-time. The change has effect after a frame is completed
    smSimulationMain *changedMain;

    volatile smInt  changedMainTimeStamp;
    /// \brief time stamp when main callback is registered
    volatile smInt  mainTimeStamp;

private:
    smInt asyncThreadPoolSize; ///< Tracks the number of threads the async threadpool is running
    /// \brief Initializes up asynchronous threadpool
    void initAsyncThreadPool();

public:
    ///initializes all the simulators in the objects in the scene..
    void init()
    {

        if (isInitialized == true)
        {
            return;
        }
        if (maxThreadCount == 0)
        {
            maxThreadCount = SIMMEDTK_MAX(simulators.size(), collisionDetectors.size());
        }
        threadPool = std::unique_ptr<ThreadPool>(new ThreadPool(maxThreadCount));

        for (smInt i = 0; i < this->simulators.size(); i++)
        {
            smObjectSimulator *objectSimulator = simulators[i];
            objectSimulator->init();
        }

        initAsyncThreadPool();

        isInitialized = true;
    }
    /// \brief constructor gets error log
    smSimulator(smErrorLog *p_log)
    {

        type = SIMMEDTK_SMSIMULATOR;
        isInitialized = false;
        this->log = p_log;
        frameCounter = 0;
        main = NULL;
        changedMain = NULL;
        changedMainTimeStamp = 0;
        mainTimeStamp = 0;
        maxThreadCount = 0;
        asyncThreadPoolSize = 0;
    }

    void setMaxThreadCount(smInt p_threadMaxCount)
    {

        if (p_threadMaxCount < 0)
        {
            return;
        }
        else
        {
            maxThreadCount = p_threadMaxCount;
        }
    }

    ///Simualtor registers the simulator and schedules it.
    ///the function is reentrant it is not thread safe.
    void registerObjectSimulator(smObjectSimulator *objectSimulator);

    void  registerCollisionDetection(smObjectSimulator *p_collisionDetection);

    ///Registration of the Simulation main. It is called in each and every frame
    void registerSimulationMain(smSimulationMain*p_main);
    /// \brief the actual implementation of the simulator module resides in run function
    void run();
    /// \brief called at the beginning of  each and every frame
    virtual void beginFrame();
    /// \brief called at the end of each and every frame
    virtual void endFrame();
    /// \brief this is called by SDK. it lanuches the simulator module
    virtual void exec()
    {

        if (isInitialized)
        {
            run();
        }
        else
        {
            init();
            run();
        }
    }
};

#endif
