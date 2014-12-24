/*
****************************************************
                SimMedTK LICENSE
****************************************************

****************************************************
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