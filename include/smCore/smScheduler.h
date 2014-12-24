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

#ifndef SMSCHEDULER_H
#define SMSCHEDULER_H

class smSimulator;

enum smSchedulingPriority
{
    SIMMEDTK_SCHEDULING_HIGHRATE,
    SIMMEDTK_SCHEDULING_AUTOSCHEDULE,
    SIMMEDTK_SCHEDULING_BACKGROUND
};

struct smScheduleGroup
{

#ifdef SIMMEDTK_OPERATINGSYSTEM_WINDOWS
    HANDLE threadHandle;
#endif

public:
    smFloat groupWeight;
    smShort totalThreads;
    smFloat maxObservedFPS;
    smFloat minObservedFPS;
    smShort minObservedFPSThreadIndex;
    smShort maxObservedFPSThreadIndex;

    smScheduleGroup()
    {
        totalThreads = 1;
        minTargetFPS = maxTargetFPS = maxObservedFPS = minObservedFPS = 0;

        minObservedFPSThreadIndex = maxObservedFPSThreadIndex = 0;
#ifdef SIMMEDTK_OPERATINGSYSTEM_WINDOWS
        threadHandle = GetCurrentThread();
#endif
    }
    smFloat maxTargetFPS;
    smFloat minTargetFPS;
    friend class smSimulator;
};

#endif