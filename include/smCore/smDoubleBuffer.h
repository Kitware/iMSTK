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

#ifndef SMDOUBLEBUFFER_H
#define SMDOUBLEBUFFER_H

#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smUtilities/smDataStructs.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>
#include <condition_variable>

#define SIMMEDTK_PIPE_MAXLISTENERS 10

template <class Template>
struct smBuffer
{
    smInt  frameCounter;
    smInt totalElements;
    Template*buffer;
};

enum  smDoubleBufferReturn
{
    SIMMEDTK_DOUBLEBUFFER_NONEWDATA,
    SIMMEDTK_DOUBLEBUFFER_NOMEMORY,
    SIMMEDTK_DOUBLEBUFFER_OK
};

///this class implementes memory buffer for multiple threads where there exists at least one writer
///and one reader thread exists
template <class Template>
class smDoubleBuffer: public smCoreClass
{
    ///buffers one for reading, one for writing
    smBuffer<Template> buffer[2];

    ///buffer Size
    smInt bufferSize;

    ///indicates the reader buffer
    smInt readerBuffer;

    ///indicates the writer buffer
    smInt writerBuffer;

    ///buffer access lock. This is used for the swapping the buffer or during the memory copy
    ///operation
    std::mutex bufferLock;

    ///wait condition is triggered when anyone waits the buffer to written. When the memory
    ///writing is completed then the event is triggered, any waiting
    std::condition_variable readBufferReady;

    ///if one thread reads
    smInt lastReadFrameCounter;

private:
    ///swap buffers
    void swapBuffers()
    {
        std::unique_lock<std::mutex> uniLock(bufferLock, std::defer_lock);
        uniLock.lock();
        buffer[writerBuffer].frameCounter++;
        readerBuffer = (++readerBuffer) % 2;
        writerBuffer = (++writerBuffer) % 2;
        buffer[writerBuffer].totalElements = 0;
        bufferLock.unlock();
        readBufferReady.notify_all();
    }

public:
    smDoubleBuffer()
    {
        type = SIMMEDTK_SMDOUBLEBUFFER;
    }

    /// \param p_bufferSize buffer size of the read/write buffers
    smDoubleBuffer(smInt p_bufferSize)
    {
        smDoubleBuffer();

        buffer[0].totalElements = 0;
        buffer[0].frameCounter = 0;
        buffer[0].buffer = new Template[p_bufferSize];
        writerBuffer = 0;

        buffer[1].totalElements = 0;
        buffer[1].frameCounter = 0;
        buffer[1].buffer = new Template[p_bufferSize];
        bufferSize = p_bufferSize;
        readerBuffer = 1;

        lastReadFrameCounter = 0;
    }


    ///this funtcion must be called before starting
    Template* beginWrite()
    {
        return (buffer[writerBuffer].buffer);
    }

    ///completes writing process by setting number of elements written and swaps the buffer
    void endWrite(smInt totalElements)
    {
        buffer[writerBuffer].totalElements = (totalElements > bufferSize ? bufferSize : totalElements);
        swapBuffers();
    }

    ///copy buffer by checking the last read frame. If there is no new data it will return.
    ///if you need only latest available data, call this function it is fast.
    ///notice that this call is for reentrant calls of the the same reader thread
    smDoubleBufferReturn copyAvailableBuffer(Template **p_T, smInt &p_number, smInt lastRead)
    {
        smDoubleBufferReturn ret;
        std::lock_guard<std::mutex> lock(bufferLock); //Lock is released when leaves scope

        if (buffer[readerBuffer].frameCounter > lastRead)
        {
            memcpy(*p_T, buffer[readerBuffer].buffer, buffer[readerBuffer].totalElements);
            p_number = buffer[readerBuffer].totalElements;
            lastReadFrameCounter = buffer[readerBuffer].frameCounter;
            ret = SIMMEDTK_DOUBLEBUFFER_OK;
        }
        else
        {
            ret = SIMMEDTK_DOUBLEBUFFER_NONEWDATA;
        }

        return ret;
    }
    ///the same functionality as
    smInt  copyAvailableBuffer(Template **p_T, smInt &p_number)
    {
        smInt frameCounterReader;
        std::lock_guard<std::mutex> lock(bufferLock); //Lock is released when leaves scope

        memcpy(*p_T, buffer[readerBuffer].buffer, buffer[readerBuffer].totalElements);
        p_number = buffer[readerBuffer].totalElements;
        frameCounterReader = buffer[readerBuffer].frameCounter;

        return frameCounterReader;
    }

    /// \brief
    ///         it will wait the latest data to be available. When the data is available, it will copy to the
    ///         pointed block by p_T
    /// \param p_T it is the location for writing the memory block.
    /// \param p_number has the total elements in the reader buffer. It is returned to the caller
    void copyLatestBuffer(Template *p_T, smInt &p_number)
    {
        std::unique_lock<std::mutex> uniLock(bufferLock, std::defer_lock);
        uniLock.lock();
        readBufferReady.wait(uniLock);
        memcpy(p_T, buffer[readerBuffer].buffer, buffer[readerBuffer].totalElements);
        p_number = buffer[readerBuffer].totalElements;
        lastReadFrameCounter = buffer[readerBuffer].frameCounter;
        uniLock.unlock();
    }
};

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
    smPipeData()
    {
        dataLocation = NULL;
        dataReady = false;
        nbrElements = 0;
        timeStamp = 0;
    }

    void *dataLocation;//if the data is call-by-value data is written here
    volatile smInt  nbrElements;
    volatile smBool dataReady;///flag is raised when the data is ready
    volatile smUInt timeStamp;
};
/// \brief pipe registration holder
struct smPipeRegisteration
{
    /// \brief constructor
    smPipeRegisteration()
    {
        regType = SIMMEDTK_PIPE_BYREF;
    }

    smPipeRegisteration(smPipeRegType p_reg)
    {
        regType = p_reg;
    }

    smCoreClass *listenerObject;///pointer to the listener for future use
    smPipeData  data;///information about the data.
    smPipeRegType regType;///registration type. Will it be reference or value registration.
    /// \brief  print pipe registration information
    void print()
    {
        if (regType == SIMMEDTK_PIPE_BYREF)
        {
            std::cout << "Listener Object" << " By Reference" << "\n";
        }

        if (regType == SIMMEDTK_PIPE_BYVALUE)
        {
            std::cout << "Listener Object" << " By Value" << "\n";
        }
    }

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
    ~smPipe()
    {
        smIndiceArrayIter<smPipeRegisteration*> iterValue(&byValue);

        for (smInt i = iterValue.begin(); i < iterValue.end(); i++)
        {
            delete []iterValue[i]->data.dataLocation;
        }
    }
    /// \brief pipe constructor
    smPipe(smString p_name, smInt p_elementSize, smInt p_maxElements,
           smPipeType p_pipeType = SIMMEDTK_PIPE_TYPEANY):
        byRefs(SIMMEDTK_PIPE_MAXLISTENERS),
        byValue(SIMMEDTK_PIPE_MAXLISTENERS)
    {
        name = p_name;
        maxElements = p_maxElements;
        elementSize = p_elementSize;
        data = new smChar[elementSize * maxElements]; //change it to memory block later on
        pipeType = p_pipeType;
    }
    /// \brief get maximum number of elements
    inline smInt getElements()
    {
        return maxElements;
    }
    /// \brief  begin writing
    inline volatile void *beginWrite()
    {
        return data;
    }
    /// \brief  end writing
    inline void endWrite(smInt p_elements)
    {
        currentElements = p_elements;
        timeStamp++;
        acknowledgeRefListeners();
    }
    /// \brief  register for the pipe
    inline smInt registerListener(smPipeRegisteration *p_pipeReg)
    {
        if (p_pipeReg->regType == SIMMEDTK_PIPE_BYREF)
        {
            p_pipeReg->data.dataLocation = data;
            return byRefs.add(p_pipeReg);
        }
        else if (p_pipeReg->regType == SIMMEDTK_PIPE_BYVALUE)
        {
            p_pipeReg->data.dataLocation = new smChar[elementSize * maxElements];
            p_pipeReg->data.dataReady = false;
            p_pipeReg->data.nbrElements = 0;
            p_pipeReg->data.timeStamp = timeStamp;
            return byValue.add(p_pipeReg);
        }
    }

    ///Acknowledge only raises the flag so that it will enable the listeners
    inline void acknowledgeRefListeners()
    {
        smIndiceArrayIter<smPipeRegisteration*> iterRef(&byRefs);

        for (smInt i = iterRef.begin(); i < iterRef.end(); i++)
        {
            (iterRef[i]->data.nbrElements) = currentElements;
            (iterRef[i]->data.timeStamp) = timeStamp;
            (iterRef[i]->data.dataReady) = true;
        }
    }

    ///This is for copy-by-value listeners..With this, the data will be copied to the listener provided data location
    inline void acknowledgeValueListeners()
    {
        smIndiceArrayIter<smPipeRegisteration*> iter(&byValue);

        for (smInt i = iter.begin(); i < iter.end(); i++)
        {
            memcpy(iter[i]->data.dataLocation, data, currentElements * elementSize);
            (iter[i]->data.nbrElements) = currentElements;
            (iter[i]->data.dataReady) = true;
        }
    }

    ///For copy by value, the function checks
    inline void checkAndCopyNewData(smInt p_handleByValue)
    {
        smPipeRegisteration *reg = byValue.getByRef(p_handleByValue);

        if (reg->data.timeStamp < timeStamp)
        {
            memcpy(reg->data.dataLocation, data, currentElements * elementSize);
        }

        reg->data.nbrElements = currentElements;
    }

    ///for copy by value usage
    inline void copyData(smInt p_handleByValue)
    {
        memcpy(byValue.getByRef(p_handleByValue)->data.dataLocation, data, currentElements * elementSize);
        (byValue.getByRef(p_handleByValue)->data.nbrElements) = currentElements;
    }

    friend smBool operator==(smPipe &p_pipe, smString p_name)
    {
        return (p_pipe.name == p_name ? true : false);
    }
    /// \brief print all the listeners (both reference  and value)
    void print()
    {
        byRefs.print();
        byValue.print();
    }
};
/// \brief secure pipe is for consumer-producer relation. not implemented yet
class smSecurePipe: public smPipe
{

};

#endif
