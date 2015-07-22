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

#ifndef SMWORKERTHREAD_H
#define SMWORKERTHREAD_H

// SimMedTK includes
#include "Core/CoreClass.h"
#include "Core/Synchronization.h"

/// \brief process id
struct ProcessID
{
public:

    /// \brief process(used as conceptual meaning) numbering scheme
    enum class ProcessNumbering
    {
        X,
        XY,
        XYZ,
    };

    /// \brief numbering scheme in x,y,z
    unsigned short x;
    unsigned short y;
    unsigned short z;

    unsigned short totalProcX;
    unsigned short totalProcY;
    unsigned short totalProcZ;
    /// \brief data
    void *data;
    /// \brief data size
    int sizeOfData;
    /// \brief numbering sceheme
    ProcessNumbering numbScheme;

    ProcessID()
    {
        x = y = z = totalProcX = totalProcY = totalProcZ = sizeOfData = 0;
        data = NULL;
        numbScheme = ProcessNumbering::X;
    }

    inline void operator=(ProcessID p_ID)
    {
        x = p_ID.x;
        y = p_ID.y;
        z = p_ID.z;
        totalProcX = p_ID.totalProcX;
        totalProcY = p_ID.totalProcY;
        totalProcZ = p_ID.totalProcZ;

        data = new char[p_ID.sizeOfData];
        memcpy(data, p_ID.data, sizeOfData);
    }

};



/// \brief process. Process is a atomic execution unit(thread).
class Process: public CoreClass
{

protected:
    ProcessID id;
    bool termination;

public:
    Process()
    {
        id.x = 0;
        id.y = 0;
        id.z = 0;
        id.totalProcX = 0;
        id.totalProcY = 0;
        id.totalProcZ = 0;
        id.data = NULL;
        id.sizeOfData = 0;
        id.numbScheme = ProcessID::ProcessNumbering::X;
        termination = false;
    }

    Process(ProcessID p_id)
    {
        id = p_id;
        termination = false;
    }

    void setId(ProcessID p_id)
    {
        id = p_id;
    }

    virtual void kernel() = 0;

    void terminate()
    {
        termination = true;
    }
};
/// \brief worker thread extends process
class WorkerThread: public Process
{

protected:
    /// \brief for synchronization
    Synchronization *synch;

public:
    WorkerThread()
    {
    }

    WorkerThread(ProcessID p_ID): Process(p_ID)
    {
        termination = false;
    }

    WorkerThread(Synchronization &p_synch, ProcessID p_ID): Process(p_ID)
    {
        synch = &p_synch;
        termination = false;
    }

    void setSynchObject(Synchronization &p_synch)
    {
        synch = &p_synch;
    }

    virtual void run() = 0;
};

#endif