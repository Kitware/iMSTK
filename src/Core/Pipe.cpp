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

#include "Pipe.h"

smPipeData::smPipeData()
{
    dataLocation = NULL;
    dataReady = false;
    nbrElements = 0;
    timeStamp = 0;
}
smPipeRegisteration::smPipeRegisteration()
{
    regType = SIMMEDTK_PIPE_BYREF;
}
smPipeRegisteration::smPipeRegisteration ( smPipeRegType p_reg )
{
    regType = p_reg;
}
void smPipeRegisteration::print()
{
    if ( regType == SIMMEDTK_PIPE_BYREF )
    {
        std::cout << "Listener Object" << " By Reference" << "\n";
    }

    if ( regType == SIMMEDTK_PIPE_BYVALUE )
    {
        std::cout << "Listener Object" << " By Value" << "\n";
    }
}
smPipe::~smPipe()
{
    smIndiceArrayIter<smPipeRegisteration*> iterValue ( &byValue );

    for ( smInt i = iterValue.begin(); i < iterValue.end(); i++ )
    {
        delete [] static_cast<smChar*>(iterValue[i]->data.dataLocation);
    }
}
smPipe::smPipe ( std::string p_name, int p_elementSize, int p_maxElements, smPipeType p_pipeType ) :
    byRefs ( SIMMEDTK_PIPE_MAXLISTENERS ),
    byValue ( SIMMEDTK_PIPE_MAXLISTENERS )
{
    name = p_name;
    maxElements = p_maxElements;
    elementSize = p_elementSize;
    data = new smChar[elementSize * maxElements]; //change it to memory block later on
    pipeType = p_pipeType;
}
int smPipe::getElements()
{
    return maxElements;
}
void* smPipe::beginWrite()
{
    return data;
}
void smPipe::endWrite ( int p_elements )
{
    currentElements = p_elements;
    timeStamp++;
    acknowledgeRefListeners();
}
int smPipe::registerListener ( smPipeRegisteration* p_pipeReg )
{
    if ( p_pipeReg->regType == SIMMEDTK_PIPE_BYREF )
    {
        p_pipeReg->data.dataLocation = data;
        return byRefs.add ( p_pipeReg );
    }
    p_pipeReg->data.dataLocation = new smChar[elementSize * maxElements];
    p_pipeReg->data.dataReady = false;
    p_pipeReg->data.nbrElements = 0;
    p_pipeReg->data.timeStamp = timeStamp;
    return byValue.add ( p_pipeReg );
}
void smPipe::acknowledgeRefListeners()
{
    smIndiceArrayIter<smPipeRegisteration*> iterRef ( &byRefs );

    for ( smInt i = iterRef.begin(); i < iterRef.end(); i++ )
    {
        ( iterRef[i]->data.nbrElements ) = currentElements;
        ( iterRef[i]->data.timeStamp ) = timeStamp;
        ( iterRef[i]->data.dataReady ) = true;
    }
}
void smPipe::acknowledgeValueListeners()
{
    smIndiceArrayIter<smPipeRegisteration*> iter ( &byValue );

    for ( smInt i = iter.begin(); i < iter.end(); i++ )
    {
        memcpy ( iter[i]->data.dataLocation, data, currentElements * elementSize );
        ( iter[i]->data.nbrElements ) = currentElements;
        ( iter[i]->data.dataReady ) = true;
    }
}
void smPipe::checkAndCopyNewData ( int p_handleByValue )
{
    smPipeRegisteration *reg = byValue.getByRef ( p_handleByValue );

    if ( reg->data.timeStamp < timeStamp )
    {
        memcpy ( reg->data.dataLocation, data, currentElements * elementSize );
    }

    reg->data.nbrElements = currentElements;
}
void smPipe::copyData ( int p_handleByValue )
{
    memcpy ( byValue.getByRef ( p_handleByValue )->data.dataLocation, data, currentElements * elementSize );
    ( byValue.getByRef ( p_handleByValue )->data.nbrElements ) = currentElements;
}
void smPipe::print() const
{
    byRefs.print();
    byValue.print();
}
bool operator== ( smPipe& p_pipe, std::string p_name )
{
    return ( p_pipe.name == p_name ? true : false );
}
