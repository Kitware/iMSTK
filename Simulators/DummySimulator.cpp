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
#include "Simulators/DummySimulator.h"
#include "Core/StaticSceneObject.h"
#include "Core/Event.h"
#include "Event/KeyboardEvent.h"
#include "Collision/MeshCollisionModel.h"

DummySimulator::DummySimulator( std::shared_ptr<ErrorLog> p_errorLog ) : ObjectSimulator( p_errorLog )
{
}
void DummySimulator::beginSim()
{
    //start the job
}
void DummySimulator::initCustom()
{
    //do nothing for now
    for ( size_t i = 0; i < objectsSimulated.size(); i++ )
    {
        auto object = objectsSimulated[i];

        switch ( object->getType() )
        {
            case core::ClassType::StaticSceneObject:
            {
                auto staticObject = std::static_pointer_cast<StaticSceneObject>(object);
                auto model = std::static_pointer_cast<MeshCollisionModel>(staticObject->getModel());
                if(nullptr == model)
                {
                    break;
                }
                std::shared_ptr<Mesh> mesh = model->getMesh();

                object->getLocalVertices().reserve( mesh->nbrVertices );
                // WARNING:  Copy!!?
                object->getLocalVertices() = mesh->vertices;
                object->getFlags().isSimulatorInit = true;
                break;
            }
            default:
                std::cerr << "Unknown scene object type." << std::endl;

        }
    }
}
void DummySimulator::run()
{
    beginSim();

    for ( size_t i = 0; i < this->objectsSimulated.size(); i++ )
    {
        auto sceneObj = this->objectsSimulated[i];

        //ensure that dummy simulator will work on static scene objects only.
        if ( sceneObj->getType() == core::ClassType::StaticSceneObject )
        {
            auto staticSceneObject = std::static_pointer_cast<StaticSceneObject>(sceneObj);
            auto model = std::static_pointer_cast<MeshCollisionModel>(staticSceneObject->getModel());
            if(nullptr == model)
            {
                break;
            }
            std::shared_ptr<Mesh> mesh = model->getMesh();

            for ( int vertIndex = 0; vertIndex < mesh->nbrVertices; vertIndex++ )
            {
                staticSceneObject->getLocalVertices()[vertIndex][1] = staticSceneObject->getLocalVertices()[vertIndex][1] + 0.000001;
            }
        }
    }

    endSim();
}
void DummySimulator::endSim()
{
    //end the job
}
void DummySimulator::syncBuffers()
{
    for ( size_t i = 0; i < this->objectsSimulated.size(); i++ )
    {
        auto sceneObj = this->objectsSimulated[i];

        //ensure that dummy simulator will work on static scene objects only.
        if ( sceneObj->getType() == core::ClassType::StaticSceneObject )
        {
            auto staticSceneObject = std::static_pointer_cast<StaticSceneObject>(sceneObj);
            auto model = std::static_pointer_cast<MeshCollisionModel>(staticSceneObject->getModel());
            if(nullptr == model)
            {
                break;
            }
            std::shared_ptr<Mesh> mesh = model->getMesh();
            // WARNING: Copy??!
            mesh->vertices = staticSceneObject->getLocalVertices();
        }
    }
}
void DummySimulator::handleEvent(std::shared_ptr<core::Event> p_event )
{
    if(!this->isListening())
    {
        return;
    }

    auto keyboardEvent = std::static_pointer_cast<event::KeyboardEvent>(p_event);
    if(keyboardEvent)
    {
        switch(keyboardEvent->getKeyPressed())
        {
            case event::Key::F1:
            {
                std::cout << "F1 Keyboard is pressed " ;//<< keyboardEvent->getKeyPressed() << std::endl;
                break;
            }
            default:
                break;
        }
    }
}
