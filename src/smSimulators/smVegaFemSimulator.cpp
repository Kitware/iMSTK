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

// SimMedTK includes
#include "smSimulators/smVegaFemSimulator.h"

smVegaFemSimulator::smVegaFemSimulator( std::shared_ptr<smErrorLog> p_errorLog ) : smObjectSimulator( p_errorLog )
{
    hapticButtonPressed = false;
}

void smVegaFemSimulator::setDispatcher( std::shared_ptr<smEventDispatcher> p_eventDispatcher )
{
    eventDispatcher = p_eventDispatcher;
}

void smVegaFemSimulator::beginSim()
{
}

void smVegaFemSimulator::initCustom()
{//do nothing for now
    for ( size_t i = 0; i < objectsSimulated.size(); i++ )
    {
        auto object = objectsSimulated[i];

        switch ( type )
        {
            case SIMMEDTK_SMVEGAFEMSCENEOBJECT:
            {
                object->getFlags().isSimulatorInit = true;
                break;
            }
            default:
                std::cerr << "Unknown class name." << std::endl;
        }
    }
}

void smVegaFemSimulator::run()
{
    beginSim();

    for ( size_t i = 0; i < this->objectsSimulated.size(); i++ )
    {
        auto sceneObj = this->objectsSimulated[i];

        //ensure that dummy simulator will work on static scene objects only.
        if ( sceneObj->getType() == SIMMEDTK_SMVEGAFEMSCENEOBJECT )
        {
            auto femSceneObject = std::static_pointer_cast<smVegaFemSceneObject>(sceneObj);
            femSceneObject->advanceDynamics();
        }
    }

    endSim();
}

void smVegaFemSimulator::endSim()
{
}

void smVegaFemSimulator::syncBuffers()
{
}

void smVegaFemSimulator::handleEvent( std::shared_ptr<smEvent> p_event )
{
    switch ( p_event->getEventType().eventTypeCode )
    {
        case SIMMEDTK_EVENTTYPE_KEYBOARD:
        {
            auto keyBoardData = std::static_pointer_cast<smKeyboardEventData>(p_event->getEventData());

            if ( keyBoardData->keyBoardKey == smKey::F1 )
            {
                printf( "F1 Keyboard is pressed %c\n", keyBoardData->keyBoardKey );
            }

            break;
        }


        case SIMMEDTK_EVENTTYPE_HAPTICOUT:
        {
            auto hapticEventData = std::static_pointer_cast<smHapticOutEventData>(p_event->getEventData());

            if ( hapticEventData->deviceId == 1 )
            {
                hapticPosition[0] = hapticEventData->position[0];
                hapticPosition[1] = hapticEventData->position[1];
                hapticPosition[2] = hapticEventData->position[2];
                hapticButtonPressed = hapticEventData->buttonState[0];
            }

            break;
        }
        default:
            std::cerr << "Unknown class name." << std::endl;

    }
}
