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

#include "smCore/smModule.h"

/// \brief Begin frame will be called before the cycle
void  smModule::beginModule()
{

    dispathcer->handle(static_cast<smCoreClass*>(this), SIMMEDTK_CALLERSTATE_BEGINFRAME);
    beginFrame();

}

/// \brief End frame will be called after the cycle
void  smModule::endModule()
{

    endFrame();
    dispathcer->handle(static_cast<smCoreClass*>(this), SIMMEDTK_CALLERSTATE_ENDFRAME);
}
void smModule::terminate()
{
    terminateExecution = true;
}
bool smModule::isTerminationDone()
{
    return terminationCompleted;
}
void smModule::waitTermination()
{
    while ( 1 )
    {
        if ( terminationCompleted == true )
        {
            break;
        }
    }
}
void smModule::setEventDispatcher( smEventDispatcher *p_dispathcer )
{
    eventDispatcher = p_dispathcer;
}
int smModule::getModuleId()
{
    return uniqueId.ID;
}
