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

#include "Core/Dispatcher.h"

Dispatcher::Dispatcher()
{
    type = core::ClassType::Dispathcer;
}

//---------------------------------------------------------------------------
core::ResultDispatcher Dispatcher::handle(std::shared_ptr<CoreClass> p_caller, core::CallerState p_callerState)
{

    core::ClassType classType;

    //get the type of the class which is calling
    classType = p_caller->getType();

    switch (classType)
    {
        //handle for viewer
    case core::ClassType::Viewer:
        handleViewer(p_caller, p_callerState);
        break;
        //handle for simulator
    case core::ClassType::Simulator:
        handleSimulator(p_caller, p_callerState);
        break;
        //handle for collision detection
    case core::ClassType::CollisionDetection:
        handleCollisionDetection(p_caller, p_callerState);
        break;
    default:
        std::cerr << "Unknown class type" << std::endl;
    }

    handleAll();
    return core::ResultDispatcher::Success;
}

//---------------------------------------------------------------------------
void Dispatcher::handleViewer(std::shared_ptr<CoreClass> /*p_caller*/,
                              core::CallerState /*p_callerState*/)
{

}

//---------------------------------------------------------------------------
void Dispatcher::handleSimulator(std::shared_ptr<CoreClass> /*p_caller*/,
                                 core::CallerState /*p_callerState*/)
{

}

//---------------------------------------------------------------------------
void Dispatcher::handleCollisionDetection(std::shared_ptr<CoreClass> /*p_caller*/,
                                          core::CallerState /*p_callerState*/)
{

}

//---------------------------------------------------------------------------
void Dispatcher::handleCollisionResponse(std::shared_ptr<CoreClass> /*p_caller*/,
                                         core::CallerState /*p_callerState*/)
{

}

//---------------------------------------------------------------------------
void Dispatcher::handleAll()
{

}
