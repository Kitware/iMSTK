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

// STL includes
#include <memory>

// SimMedTK includes
#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smCore/smSceneObject.h"
#include "smCore/smScheduler.h"
#include "smCore/smErrorLog.h"
#include "smUtilities/smTimer.h"

/// \brief  thread priority definitions
enum smThreadPriority
{
    SIMMEDTK_THREAD_IDLE,
    SIMMEDTK_THREAD_LOWPRIORITY,
    SIMMEDTK_THREAD_NORMALPRIORITY,
    SIMMEDTK_THREAD_HIGHESTPRIORITY,
    SIMMEDTK_THREAD_TIMECRITICAL,
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
class smObjectSimulator : public smCoreClass
{

    ///friend class since smSimulator is the encapsulates the other simulators.
    friend class smSimulator;

protected:
    ///log of the object
    std::shared_ptr<smErrorLog> log;
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

    //std::shared_ptr<smUnifiedId> objectSimulatorId;
    smBool enabled;

    ///the function is reentrant it is not thread safe.
    virtual void addObject(std::shared_ptr<smSceneObject> p_object);

    /// \brief remove object from the simulator
    virtual void removeObject(std::shared_ptr<smSceneObject> p_object);

    smObjectSimulator(std::shared_ptr<smErrorLog> p_log);

    /// \brief  set thread priority
    void setPriority(smThreadPriority p_priority);
    /// \brief  set execution type
    void setExecutionType(smSimulatorExecutionType p_type);

    /// \brief  get thread priority
    smThreadPriority getPriority();

protected:
    ///objects that are simulated by this will be added to the list
    std::vector<std::shared_ptr<smSceneObject>> objectsSimulated;

    virtual void initCustom() = 0;

    /// \brief  init simulator
    void init();

    /// \brief  the actual implementation will reside here
    virtual void run() = 0;

    /// \brief  begining of simulator frame, this function is called
    virtual void beginSim();

    /// \brief synchronization
    virtual void syncBuffers() = 0;

    /// \brief is called at the end of simulation frame.
    virtual void endSim();

    /// \brief updates scene list
    virtual void updateSceneList();

    /// \brief  initialization routine for rendering
    virtual void initDraw(const smDrawParam &p_params);

    /// \brief  rendering of simulator. it is used for debugging purposes
    virtual void draw(const smDrawParam &p_params);

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
        smObjectSimulatorObjectIter(smScheduleGroup &p_group,
                                    std::vector<std::shared_ptr<smSceneObject>> &p_objectsSimulated,
                                    smInt p_threadIndex);

        inline void setThreadIndex(smShort p_threadIndex);

        inline smInt begin();

        inline smInt end();
    };
};

#endif
