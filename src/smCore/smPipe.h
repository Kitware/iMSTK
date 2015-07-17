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

#ifndef SMPIPE_H
#define SMPIPE_H

// STL includes
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>
#include <condition_variable>

// SimMedTK includes
#include "smConfig.h"
#include "smCoreClass.h"
#include "smDataStructures.h"

#define SIMMEDTK_PIPE_MAXLISTENERS 10

/// \brief pipe registration type such as reference or a value
enum smPipeRegType
{
    SIMMEDTK_PIPE_BYREF,
    SIMMEDTK_PIPE_BYVALUE
};
/// \brief pipe type
enum smPipeType
{
    SIMMEDTK_PIPE_TYPEREF,
    SIMMEDTK_PIPE_TYPEVALUE,
    SIMMEDTK_PIPE_TYPEANY
};
/// \brief holder for pipe data
struct smPipeData
{
    smPipeData();

    void *dataLocation;//if the data is call-by-value data is written here
    volatile smInt  nbrElements;
    volatile smBool dataReady;///flag is raised when the data is ready
    volatile smUInt timeStamp;
};
/// \brief pipe registration holder
struct smPipeRegisteration
{
    /// \brief constructor
    smPipeRegisteration();

    smPipeRegisteration(smPipeRegType p_reg);

    smCoreClass *listenerObject;///pointer to the listener for future use
    smPipeData  data;///information about the data.
    smPipeRegType regType;///registration type. Will it be reference or value registration.
    /// \brief  print pipe registration information
    void print();

};
/// \brief pipe is used for communication among the entities in the framework
class smPipe: public smCoreClass
{
protected:
    /// \brief  pipe stype
    smPipeType pipeType;
    /// \brief  number of elements
    smInt maxElements;
    /// \brief  pointer to the data
    void *data;
    /// \brief enable/disable
    smBool enabled;
    /// \brief indices to the objects that  register by reference
    smIndiceArray<smPipeRegisteration*> byRefs;
    /// \brief ndices to the objects that  register by value
    smIndiceArray<smPipeRegisteration*> byValue;
    /// \brief number of current elements in the pipe
    volatile smInt currentElements;
    volatile smUInt timeStamp;//For stamping the data
    /// \brief element size
    volatile smInt elementSize;

public:
    /// \brief  destructor
    ~smPipe();
    /// \brief pipe constructor
    smPipe(smString p_name, smInt p_elementSize, smInt p_maxElements,
           smPipeType p_pipeType = SIMMEDTK_PIPE_TYPEANY);
    /// \brief get maximum number of elements
    smInt getElements();
    /// \brief  begin writing
    void *beginWrite();
    /// \brief  end writing
    void endWrite(smInt p_elements);
    /// \brief  register for the pipe
    smInt registerListener(smPipeRegisteration *p_pipeReg);

    ///Acknowledge only raises the flag so that it will enable the listeners
    void acknowledgeRefListeners();

    ///This is for copy-by-value listeners..With this, the data will be copied to the listener provided data location
    void acknowledgeValueListeners();

    ///For copy by value, the function checks
    void checkAndCopyNewData(smInt p_handleByValue);

    ///for copy by value usage
    void copyData(smInt p_handleByValue);

    friend smBool operator==(smPipe &p_pipe, smString p_name);
    /// \brief print all the listeners (both reference  and value)
    void print() const;
};
/// \brief secure pipe is for consumer-producer relation. not implemented yet
class smSecurePipe: public smPipe
{

};

#endif // SMPIPE_H
