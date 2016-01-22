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

#ifndef SIMULATION_MANAGER_SCHEDULER_H
#define SIMULATION_MANAGER_SCHEDULER_H

class Simulator;

struct ScheduleGroup
{

#ifdef _WIN32
    HANDLE threadHandle;
#endif

public:
    float groupWeight;
    short totalThreads;
    float maxObservedFPS;
    float minObservedFPS;
    short minObservedFPSThreadIndex;
    short maxObservedFPSThreadIndex;

    ScheduleGroup()
    {
        totalThreads = 1;
        minTargetFPS = maxTargetFPS = maxObservedFPS = minObservedFPS = 0;

        minObservedFPSThreadIndex = maxObservedFPSThreadIndex = 0;
#ifdef _WIN32
        threadHandle = GetCurrentThread();
#endif
    }
    float maxTargetFPS;
    float minTargetFPS;
    friend class Simulator;
};

#endif
