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

#include "Core/Module.h"

/// \brief Begin frame will be called before the cycle
void  Module::beginModule()
{
    dispathcer->handle(std::static_pointer_cast<CoreClass>(shared_from_this()), core::CallerState::BeginFrame);
    beginFrame();

}

/// \brief End frame will be called after the cycle
void  Module::endModule()
{
    endFrame();
    dispathcer->handle(std::static_pointer_cast<CoreClass>(shared_from_this()), core::CallerState::EndFrame);
}
void Module::terminate()
{
    terminateExecution = true;
}
bool Module::isTerminationDone()
{
    return terminationCompleted;
}
void Module::waitTermination()
{
    while ( 1 )
    {
        if ( terminationCompleted == true )
        {
            break;
        }
    }
}
int Module::getModuleId()
{
    return this->getUniqueId()->getId();
}
Module::Module()
{
    terminateExecution = false;
    isInitialized = false;
    name = "Module";
}
