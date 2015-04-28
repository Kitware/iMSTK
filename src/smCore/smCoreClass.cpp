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

#include "smCore/smCoreClass.h"

std::atomic_int smUnifiedID::IDcounter;

smUnifiedID::smUnifiedID()
{
    sdkID = -1;
    machineID = -1;
}

void smUnifiedID::generateUniqueID()
{
    ID = IDcounter.fetch_add( 1 );
}

int smUnifiedID::getSDKID() const
{
    return sdkID;
}

void smUnifiedID::operator=( const smUnifiedID &p_id )
{
    ID = p_id.ID;
    sdkID = p_id.sdkID;
    machineID = p_id.machineID;
}

bool smUnifiedID::operator==( smUnifiedID &p_id )
{
    return ( ID == p_id.ID && machineID == p_id.machineID );
}

bool smUnifiedID::operator==( int &p_ID )
{
    return ( ID == p_ID );
}

bool smUnifiedID::operator!=( int &p_ID )
{
    return ( ID != p_ID );
}

smCoreClass::smCoreClass() :
    name("")
{
    drawOrder = SIMMEDTK_DRAW_BEFOREOBJECTS;
    uniqueId.generateUniqueID();
}

smClassType smCoreClass::getType() const
{
    return type;
}

smDrawParam::smDrawParam() :
    rendererObject{nullptr},
    caller{nullptr},
    viewMatrix{nullptr},
    projMatrix{nullptr},
    data{nullptr}
{
}

void smCoreClass::initDraw( const smDrawParam &/*p_params*/ )
{
}
void smCoreClass::draw( const smDrawParam &/*p_params*/ )
{
}
void smCoreClass::initSimulate( const smSimulationParam &/*p_params*/ )
{
}
void smCoreClass::simulate( const smSimulationParam &/*p_params*/ )
{
}
void smCoreClass::print() const
{
}

void smCoreClass::setName( const std::string &p_objectName )
{
    name = p_objectName;
}

std::string smCoreClass::getName() const
{
    return name;
}
