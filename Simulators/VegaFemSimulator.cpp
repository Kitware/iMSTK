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
#include "Simulators/VegaFemSimulator.h"

VegaFemSimulator::VegaFemSimulator( std::shared_ptr<ErrorLog> p_errorLog ) : ObjectSimulator( p_errorLog )
{
    hapticButtonPressed = false;
}

void VegaFemSimulator::beginSim()
{
}

void VegaFemSimulator::initCustom()
{//do nothing for now
    for ( size_t i = 0; i < objectsSimulated.size(); i++ )
    {
        auto object = objectsSimulated[i];

        switch ( object->getType() )
        {
            case core::ClassType::VegaFemSceneObject:
            {
                object->getFlags().isSimulatorInit = true;
                break;
            }
            default:
                std::cerr << "Unknown class name." << std::endl;
        }
    }
}

void VegaFemSimulator::run()
{
    beginSim();

    for ( size_t i = 0; i < this->objectsSimulated.size(); i++ )
    {
        auto sceneObj = this->objectsSimulated[i];

        //ensure that dummy simulator will work on static scene objects only.
        if ( sceneObj->getType() == core::ClassType::VegaFemSceneObject )
        {
            auto femSceneObject = std::static_pointer_cast<VegaFemSceneObject>(sceneObj);
            //std::cout << "."; std::cout.flush();
            femSceneObject->advanceDynamics();
        }
    }

    endSim();
}

void VegaFemSimulator::endSim()
{
}

void VegaFemSimulator::syncBuffers()
{
}

void VegaFemSimulator::handleEvent(std::shared_ptr<core::Event> /*p_event*/ )
{
    if (!this->isListening())
    {
        return;
    }

    /*auto hapticEvent = std::static_pointer_cast<event::HapticEvent>(p_event);
    if (hapticEvent != nullptr && hapticEvent->getDeviceId() == 1)
    {
        hapticPosition = hapticEvent->getPosition();
        hapticButtonPressed = hapticEvent->getButtonState(0);
        return;
    }*/

}
