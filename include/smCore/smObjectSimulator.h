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

#ifndef SMOBJECTSIMULATOR_H
#define SMOBJECTSIMULATOR_H

#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smCore/smMemoryBlock.h"
#include <QVector>
#include <QThread>
#include <QRunnable>
#include "smCore/smSceneObject.h"
#include "smUtilities/smTimer.h"
#include "smCore/smScheduler.h"

/// \brief  thread priority definitions
enum smThreadPriority
{
    SIMMEDTK_THREAD_IDLE = QThread::IdlePriority,
    SIMMEDTK_THREAD_LOWPRIORITY = QThread::LowPriority,
    SIMMEDTK_THREAD_NORMALPRIORITY = QThread::NormalPriority,
    SIMMEDTK_THREAD_HIGHESTPRIORITY = QThread::HighestPriority,
    SIMMEDTK_THREAD_TIMECRITICAL = QThread::TimeCriticalPriority,
};

enum smSimulatorExecutionType
{
    SIMMEDTK_SIMEXECUTION_SYNCMODE,
    SIMMEDTK_SIMEXECUTION_ASYNCMODE
};

//forward declarations
class smSceneObject;

struct smObjectSimulatorParam
{
    smShort threadIndex;
};

///This is the major object simulator. Each object simulator should derive this class.
///you want particular object simualtor to work over an object just set pointer of the object. the rest will be taken care of the simulator and object simulator.
class smObjectSimulator: public smCoreClass, QRunnable
{

    ///friend class since smSimulator is the encapsulates the other simulators.
    friend class smSimulator;

protected:
    ///log of the object
    smErrorLog *log;
    smBool isObjectSimInitialized;
    smThreadPriority threadPriority;
    smTimer timer;
    smLongDouble timerPerFrame;
    smLongDouble FPS;
    smUInt frameCounter;
    smLongDouble totalTime;
    smSimulatorExecutionType execType;
    smBool executionTypeStatusChanged;

public:
    ///This is for scheduler
    smScheduleGroup scheduleGroup;

    //smUnifiedID objectSimulatorId;
    smBool enabled;

    ///the function is reentrant it is not thread safe.
    virtual void addObject(smSceneObject *p_object)
    {
        p_object->objectSim = this;
        objectsSimulated.push_back(p_object);
    }
    /// \brief remove object from the simulator
    virtual void removeObject(smSceneObject *p_object)
    {
    }

    smObjectSimulator(smErrorLog *p_log);
    /// \brief  set thread priority
    void setPriority(smThreadPriority p_priority)
    {
        threadPriority = p_priority;
    };
    /// \brief  set execution type
    void setExecutionType(smSimulatorExecutionType p_type)
    {
        if (execType != p_type)
        {
            executionTypeStatusChanged = true;
        }

        execType = p_type;
    }
    /// \brief  get thread priority
    smThreadPriority getPriority()
    {
        return threadPriority;
    }

protected:
    ///objects that are simulated by this will be added to the list
    vector <smSceneObject*> objectsSimulated;

    virtual void initCustom() = 0;
    /// \brief  init simulator
    void init()
    {
        if (isObjectSimInitialized == false)
        {
            initCustom();
            //make the simulator true..it is initialized
            isObjectSimInitialized = true;
        }
    };
    /// \brief  the actual implementation will reside here
    virtual void run() = 0;
    /// \brief  begining of simulator frame, this function is called
    virtual void beginSim()
    {
        frameCounter++;
        timer.start();
    };
    /// \brief synchronization
    virtual void syncBuffers() = 0;
    /// \brief is called at the end of simulation frame.
    virtual void endSim()
    {
        timerPerFrame = timer.now(SIMMEDTK_TIMER_INMILLISECONDS);
        totalTime += timerPerFrame;

        if (SMTIMER_FRAME_MILLISEC2SECONDS(totalTime) > 1.0)
        {
            FPS = frameCounter;
            frameCounter = 0.0;
            totalTime = 0.0;
        }
    }
    /// \brief updates scene list
    virtual void updateSceneList()
    {
    };
    /// \brief  initialization routine for rendering
    virtual void initDraw(smDrawParam p_params);
    /// \brief  rendering of simulator. it is used for debugging purposes
    virtual void draw(smDrawParam p_params);
    /// \brief object simulator iterator. The default iteration is sequantial in the order of the insertion.
    /// custom iteration requires extension of this class.
    struct smObjectSimulatorObjectIter
    {

    private:
        smShort beginIndex;
        smShort endIndex;
        smShort currentIndex;
        smShort threadIndex;
    public:
        smObjectSimulatorObjectIter(smScheduleGroup &p_group, vector <smSceneObject*> &p_objectsSimulated, smInt p_threadIndex)
        {

            smInt objectsPerThread;
            smInt leap;
            threadIndex = p_threadIndex;
            smInt totalObjects = p_objectsSimulated.size();
            leap = (totalObjects % p_group.totalThreads);
            objectsPerThread = p_objectsSimulated.size() / (p_group.totalThreads);

            if (threadIndex == 0)
            {
                beginIndex = 0;
                endIndex = objectsPerThread + (leap != 0 ? 1 : 0);

            }
            else
            {
                beginIndex = objectsPerThread * threadIndex;

                if (threadIndex < leap && leap != 0)
                {
                    beginIndex += threadIndex;
                }
                else
                {
                    beginIndex += leap;
                }

                endIndex = beginIndex + objectsPerThread;

                if (endIndex < leap && leap != 0)
                {
                    endIndex++;
                }
            }
        }

        inline void setThreadIndex(smShort p_threadIndex)
        {
            threadIndex = p_threadIndex;
        }

        inline smInt begin()
        {
            return beginIndex;
        }

        inline smInt end()
        {
            return endIndex;
        }
    };
};

#endif
