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

#include "smCore/smDispatcher.h"


void smDispatcher::handleViewer(smCoreClass *p_caller, smCallerState p_callerState)
{

    switch (p_callerState)
    {
    }
}

void smDispatcher::handleSimulator(smCoreClass *p_caller, smCallerState p_callerState)
{

}

void smDispatcher::handleCollisionDetection(smCoreClass *p_caller, smCallerState p_callerState)
{

}

void smDispatcher::handleAll()
{

}

//// \brief this function takes care of handling of system level tasks before or after every frame
///  \param p_caller        p_caller has pointer to the object which the function is called
///  \param p_callerState   p_callerState is stores the state of the at the moment of the function called.
///                         such as whether it is the beginning of the simulator frameor end of the simulator frame for now etc..
smDispathcerResult smDispatcher::handle(smCoreClass *p_caller, smCallerState p_callerState)
{

    smClassType classType;

    //get the type of the class which is calling
    classType = p_caller->getType();

    switch (classType)
    {
        //handle for viewer
SIMMEDTK_SMVIEWER:
        handleViewer(p_caller, p_callerState);
        break;
        //handle for simulator
SIMMEDTK_SMSIMULATOR:
        handleSimulator(p_caller, p_callerState);

        break;
        //handle for collision detection
SIMMEDTK_SMCOLLISIONDETECTION:
        handleCollisionDetection(p_caller, p_callerState);

        break;
    }

    handleAll();
    return SIMMEDTK_DISPATCHER_OK;
}
