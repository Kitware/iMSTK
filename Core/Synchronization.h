// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
//
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

#ifndef CORE_SYNCHRONIZATION_H
#define CORE_SYNCHRONIZATION_H

// STL includes
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <thread>

// iMSTK includes
#include "Core/Config.h"
#include "Core/CoreClass.h"

namespace imstk {

/// \brief Synchronization class for sync the start/end of multiple threads
///simply set number of worker threads in the constructor
///then each worker threads should call waitTaskStart function when the taks
///is completed they should call signalTaskDone
class Synchronization: public CoreClass
{

    std::condition_variable taskDone;
    std::condition_variable taskStart;
    std::mutex serverMutex;
    int totalWorkers;
    int finishedWorkerCounter;
    int startedWorkerCounter;
    bool workerCounterUpdated;
    int newWorkerCounter;

public:

    /// \param p_threadsForWorkers  choose the number of worker threads
    Synchronization(int p_threadsForWorkers)
    {
        type =   ClassType::Synchronization;
        totalWorkers = p_threadsForWorkers;
        finishedWorkerCounter = 0;
        startedWorkerCounter = 0;
        workerCounterUpdated = false;
    }

    /// \brief before starting tasks worker threads should wait
    void waitTaskStart()
    {
        std::unique_lock<std::mutex> uniLock(serverMutex, std::defer_lock);
        uniLock.lock();
        startedWorkerCounter++;

        if (startedWorkerCounter == totalWorkers)
        {
            startedWorkerCounter = 0;
            taskDone.notify_all();
        }

        taskStart.wait(uniLock);
        uniLock.lock();
    }

    /// \brief when the task ends the worker should call this function
    void signalTaskDone()
    {
        std::lock_guard<std::mutex> lock(serverMutex); //Lock is released when leaves scope
        finishedWorkerCounter++;

        if (finishedWorkerCounter == totalWorkers)
        {
            finishedWorkerCounter = 0;
        }
    }

    /// \brief You could change the number of worker threads synchronization
    ///Call this function after in the main thread where orchestration is done.
    void setWorkerCounter(int p_workerCounter)
    {
        newWorkerCounter = p_workerCounter;
        workerCounterUpdated = true;
    }

    int getTotalWorkers()
    {
        return totalWorkers;
    }

    /// \brief the server thread should call this for to start exeuction of the worker threads
    void startTasks()
    {
        std::unique_lock<std::mutex> uniLock(serverMutex, std::defer_lock);
        uniLock.lock();

        if (workerCounterUpdated)
        {
            finishedWorkerCounter = newWorkerCounter;
            workerCounterUpdated = false;
        }

        taskStart.notify_all();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        taskDone.wait(uniLock);
        uniLock.unlock();
    }

    /// \brief this function is fore signalling the events after waking up the worker threads.
    void startTasksandSignalEvent(int moduleId)
    {

        Event *eventSynch;
        eventSynch = new Event();
        eventSynch->eventType = IMSTK_EVENTTYPE_SYNCH;
        eventSynch->senderId = moduleId;
        eventSynch->senderType = IMSTK_SENDERTYPE_EVENTSOURCE;

        std::unique_lock<std::mutex> uniLock(serverMutex, std::defer_lock);
        uniLock.lock();
        taskStart.notify_all();
        taskDone.wait(uniLock);
        uniLock.unlock();
    }
};

}

#endif
