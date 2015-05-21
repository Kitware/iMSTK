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
#include "smSimulators/smDummySimulator.h"
#include "smCore/smStaticSceneObject.h"
#include "smEvent/smEvent.h"
#include "smEvent/smKeyboardEvent.h"

smDummySimulator::smDummySimulator( std::shared_ptr<smErrorLog> p_errorLog ) : smObjectSimulator( p_errorLog )
{
}
void smDummySimulator::beginSim()
{
    //start the job
}
void smDummySimulator::initCustom()
{
    //do nothing for now
    for ( size_t i = 0; i < objectsSimulated.size(); i++ )
    {
        auto object = objectsSimulated[i];

        switch ( object->getType() )
        {
            case SIMMEDTK_SMSTATICSCENEOBJECT:
            {
                auto staticObject = std::static_pointer_cast<smStaticSceneObject>(object);
                object->getLocalVertices().reserve( staticObject->mesh->nbrVertices );
                // WARNING:  Copy!!?
                object->getLocalVertices() = staticObject->mesh->vertices;
                object->getFlags().isSimulatorInit = true;
                break;
            }
            default:
                std::cerr << "Unknown scene object type." << std::endl;

        }
    }
}
void smDummySimulator::run()
{
    beginSim();

    for ( size_t i = 0; i < this->objectsSimulated.size(); i++ )
    {
        auto sceneObj = this->objectsSimulated[i];

        //ensure that dummy simulator will work on static scene objects only.
        if ( sceneObj->getType() == SIMMEDTK_SMSTATICSCENEOBJECT )
        {
            auto staticSceneObject = std::static_pointer_cast<smStaticSceneObject>(sceneObj);
            auto mesh = staticSceneObject->mesh;

            for ( smInt vertIndex = 0; vertIndex < staticSceneObject->mesh->nbrVertices; vertIndex++ )
            {
                staticSceneObject->getLocalVertices()[vertIndex][1] = staticSceneObject->getLocalVertices()[vertIndex][1] + 0.000001;
            }
        }
    }

    endSim();
}
void smDummySimulator::endSim()
{
    //end the job
}
void smDummySimulator::syncBuffers()
{
    for ( size_t i = 0; i < this->objectsSimulated.size(); i++ )
    {
        auto sceneObj = this->objectsSimulated[i];

        //ensure that dummy simulator will work on static scene objects only.
        if ( sceneObj->getType() == SIMMEDTK_SMSTATICSCENEOBJECT )
        {
            auto staticSceneObject = std::static_pointer_cast<smStaticSceneObject>(sceneObj);
            auto mesh = staticSceneObject->mesh;
            // WARNING: Copy??!
            mesh->vertices = staticSceneObject->getLocalVertices();
        }
    }
}
void smDummySimulator::handleEvent(std::shared_ptr<smtk::Event::smEvent> p_event )
{
    if(!this->isListening())
    {
        return;
    }

    auto keyboardEvent = std::static_pointer_cast<smtk::Event::smKeyboardEvent>(p_event);
    if(keyboardEvent)
    {
        switch(keyboardEvent->getKeyPressed())
        {
            case smtk::Event::smKey::F1:
            {
                std::cout << "F1 Keyboard is pressed " ;//<< keyboardEvent->getKeyPressed() << std::endl;
                break;
            }
            default:
                break;
        }
    }
}
