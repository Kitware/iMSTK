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

#ifndef SMMEMORYBLOCK_H
#define SMMEMORYBLOCK_H

#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smUtilities/smVec3.h"
#include "smCore/smErrorLog.h"
#include <QHash>

enum smMemReturnType
{
    SIMMEDTK_MEMORY_ALLOCATED,
    SIMMEDTK_MEMORY_ALREADYALLOCATED,
    SIMMEDTK_MEMORY_NOTENOUGHMEMORY,
    SIMMEDTK_MEMORY_MEMORYFOUND,
    SIMMEDTK_MEMORY_NOMEMORYFOUND,
    SIMMEDTK_MEMORY_INVALIDPARAMS,
    SIMMEDTK_MEMORY_INVALIDMEMORY,
    SIMMEDTK_MEMORY_NOERROR
};

/// \brief Memory Block makes easy to allocate and associate particular memory.
///        it facilities the memory mamangement
class smMemoryBlock: public smCoreClass
{

private:
    /// \brief error log for reporting the error
    smErrorLog *log;
    QHash<QString , void*> memoryBlocks;

public:
    ///constructr needs logger in case
    smMemoryBlock(smErrorLog *log)
    {
        type = SIMMEDTK_SMMEMORYBLOCK;
        this->log = log;
    }

    smMemoryBlock()
    {
        type = SIMMEDTK_SMMEMORYBLOCK;
        //this->log=smSDK::getErrorLog();
    }

    /// \brief alocate a class and returns p_returnedBlock as allocated memory and
    ///return params are SIMMEDTK_MEMORY_ALLOCATED or SIMMEDTK_MEMORY_ALREADYALLOCATED or SIMMEDTK_MEMORY_INVALIDMEMORY
    template<class T>
    smMemReturnType allocate(QString &p_memoryBlockName, T**p_returnedBlock)
    {
        *p_returnedBlock = new T();

        if (p_returnedBlock == NULL)
        {
            return    SIMMEDTK_MEMORY_NOTENOUGHMEMORY;
        }

        if (memoryBlocks.contains(p_memoryBlockName))
        {
            delete [] *p_returnedBlock;
            return SIMMEDTK_MEMORY_ALREADYALLOCATED;
        }
        else
        {
            memoryBlocks[p_memoryBlockName] = *p_returnedBlock;
            return  SIMMEDTK_MEMORY_ALLOCATED;
        }
    }

    /// \brief alocate any c;asses and returns p_returnedBlock as allocated memory and
    ///return params are SIMMEDTK_MEMORY_ALLOCATED or SIMMEDTK_MEMORY_ALREADYALLOCATED or SIMMEDTK_MEMORY_INVALIDMEMORY
    template<class T>
    smMemReturnType allocate(QString &p_memoryBlockName, smInt nbr, T**p_returnedBlock)
    {
        *p_returnedBlock = new T[nbr];

        if (p_returnedBlock == NULL)
        {
            return    SIMMEDTK_MEMORY_NOTENOUGHMEMORY;
        }

        if (memoryBlocks.contains(p_memoryBlockName))
        {
            delete [] *p_returnedBlock;
            return SIMMEDTK_MEMORY_ALREADYALLOCATED;
        }
        else
        {
            memoryBlocks[p_memoryBlockName] = *p_returnedBlock;
            return  SIMMEDTK_MEMORY_ALLOCATED;
        }
    }

    /// \brief alocate any classes and returns p_returnedBlock as allocated memory
    ///    returns   SIMMEDTK_MEMORY_ALLOCATED or SIMMEDTK_MEMORY_ALREADYALLOCATED or SIMMEDTK_MEMORY_INVALIDMEMORY
    template <class T>
    smMemReturnType allocate(const QString& p_memoryBlockName, const smInt &p_nbr)
    {
        T *allocatedMem;
        allocatedMem = new T[p_nbr];

        if (allocatedMem == NULL)
        {
            return    SIMMEDTK_MEMORY_NOTENOUGHMEMORY;
        }

        if (memoryBlocks.contains(p_memoryBlockName))
        {
            delete []allocatedMem;
            return SIMMEDTK_MEMORY_ALREADYALLOCATED;
        }
        else
        {
            memoryBlocks[p_memoryBlockName] = allocatedMem;
            return  SIMMEDTK_MEMORY_ALLOCATED;
        }
    }

    /// \brief alocate vectors and returns  SIMMEDTK_MEMORY_INVALIDPARAMS or SIMMEDTK_MEMORY_ALLOCATED based on block size given
    virtual smMemReturnType allocate(const QString &p_memoryBlockName, const smInt blockSize, void **p_returnedBlock)
    {
        if (blockSize <= 0)
        {
            return SIMMEDTK_MEMORY_INVALIDPARAMS;
        }

        *p_returnedBlock = new smChar[blockSize];
        memoryBlocks[p_memoryBlockName] = *p_returnedBlock;
        return SIMMEDTK_MEMORY_ALLOCATED;
    }

    /// \brief deletes the block from memeory as well as in has container
    virtual smMemReturnType deleteMemory(QString & p_memoryBlockName)
    {
        void *memoryBlock;

        if (memoryBlocks.contains(p_memoryBlockName))
        {
            memoryBlock = memoryBlocks[p_memoryBlockName];
            delete []memoryBlock;
            memoryBlocks.remove(p_memoryBlockName);
            return SIMMEDTK_MEMORY_NOERROR;
        }
        else
        {
            return SIMMEDTK_MEMORY_NOMEMORYFOUND;
        }
    }

    /// \brief  gets  memory from the container via given block name. it returns SIMMEDTK_MEMORY_MEMORYFOUND or SIMMEDTK_MEMORY_NOMEMORYFOUND.
    virtual smMemReturnType getBlock(const QString &p_memoryBlockName, void **p_memoryPointer)
    {
        if (memoryBlocks.contains(p_memoryBlockName))
        {
            *p_memoryPointer = memoryBlocks[p_memoryBlockName];
            return  SIMMEDTK_MEMORY_MEMORYFOUND;
        }
        else
        {
            return SIMMEDTK_MEMORY_NOMEMORYFOUND;
        }
    }

    /// \brief copy the allocated memory location to dst. p_nbr is the number of elements to be copied
    template<class T>
    smMemReturnType localtoOriginalBlock(const QString &p_memoryBlockName, T* dst, const smInt p_nbr)
    {

        T *src;

        if (dst != NULL)
        {
            src = (T*)memoryBlocks[p_memoryBlockName];

            if (src != NULL)
            {
                memcpy(dst, src, sizeof(T)*p_nbr);
                return  SIMMEDTK_MEMORY_NOERROR;
            }
            else
            {
                return SIMMEDTK_MEMORY_INVALIDMEMORY;
            }
        }
        else
        {
            return SIMMEDTK_MEMORY_INVALIDPARAMS;
        }
    }

    /// \brief copy the src to allocated memory location. p_nbr is the number of elements
    template<class T>
    smMemReturnType originaltoLocalBlock(const QString &p_memoryBlockName, T *src,
                                         const smInt p_nbr)
    {

        T *dst;

        if (src != NULL)
        {
            dst = (T*)memoryBlocks[p_memoryBlockName];

            if (dst != NULL)
            {
                memcpy(dst, src, sizeof(T)*p_nbr);
                return  SIMMEDTK_MEMORY_NOERROR;
            }
            else
            {
                return    SIMMEDTK_MEMORY_INVALIDMEMORY;
            }
        }
        else
        {
            return SIMMEDTK_MEMORY_INVALIDPARAMS;
        }
    }

    /// \brief lists the blocks within the container
    void listofBlocks()
    {
        QHash<QString , void*>::iterator iter = memoryBlocks.begin();

        while (iter != memoryBlocks.end())
        {
            cout << iter.value() << endl;
            ++iter;
        }
    }

    /// \brief  the destructor that deletes all the allocated memory locations
    ~smMemoryBlock()
    {
        foreach(void*memoryPtr, memoryBlocks)
        delete []memoryPtr;
    }

};

#endif
