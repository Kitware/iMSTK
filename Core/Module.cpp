// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
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
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#include "Core/Module.h"
#include "Core/Dispatcher.h"
#include "Core/Scene.h"

Module::Module() :
    isInitialized(false),
    terminateExecution(false),
    terminationCompleted(false)
{
    this->name = "Module";
}

//---------------------------------------------------------------------------
Module::~Module() {}

//---------------------------------------------------------------------------
void Module::terminate()
{
    this->terminateExecution = true;
}

//---------------------------------------------------------------------------
bool Module::isTerminationDone()
{
    return this->terminationCompleted;
}

//---------------------------------------------------------------------------
bool Module::isTerminated()
{
    return this->terminateExecution;
}

//---------------------------------------------------------------------------
void Module::waitTermination()
{
    while (1)
    {
        //std::cout << this->name << std::endl;
        if (true == isTerminationDone())
        {
            break;
        }
    }
}

//---------------------------------------------------------------------------
short int Module::getModuleId()
{
    return this->getUniqueId()->getId();
}

//---------------------------------------------------------------------------
void  Module::beginModule()
{
    this->dispathcer->handle(std::static_pointer_cast<CoreClass>(shared_from_this()),
                             core::CallerState::BeginFrame);
    this->beginFrame();
}

//---------------------------------------------------------------------------
void  Module::endModule()
{
    this->endFrame();
    this->dispathcer->handle(std::static_pointer_cast<CoreClass>(shared_from_this()),
                             core::CallerState::EndFrame);
}
