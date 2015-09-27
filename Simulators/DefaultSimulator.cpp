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
#include "Simulators/DefaultSimulator.h"
#include "Core/StaticSceneObject.h"
#include "Core/Event.h"
#include "Event/KeyboardEvent.h"
#include "Collision/MeshCollisionModel.h"
#include "Devices/VRPNForceDevice.h"
#include "VirtualTools/ToolCoupler.h"

DefaultSimulator::DefaultSimulator( std::shared_ptr<ErrorLog> p_errorLog ) : ObjectSimulator( p_errorLog )
{
    this->addOperation([](std::vector<core::Vec3d>& array)
    {
        for(auto &x : array)
        {
            x.array() += 0.000001;
        }
    });
}
void DefaultSimulator::beginSim()
{
    //start the job
}
void DefaultSimulator::initCustom()
{
    //do nothing for now
    for ( size_t i = 0; i < objectsSimulated.size(); i++ )
    {
        auto object = objectsSimulated[i];

        switch ( object->getType() )
        {
            case core::ClassType::StaticSceneObject:
            {
                auto sceneObject = std::static_pointer_cast<StaticSceneObject>(object);
                auto model = std::dynamic_pointer_cast<MeshCollisionModel>(sceneObject->getModel());
                if(!model)
                {
                    std::cerr << "Unknown model type." << std::endl;
                    break;
                }
                auto mesh = model->getMesh();

                object->getLocalVertices().reserve( mesh->getNumberOfVertices() );
                // WARNING:  Copy!!?
                object->getLocalVertices() = mesh->getVertices();
                object->getFlags().isSimulatorInit = true;
                break;
            }
            default:
                std::cerr << "Unknown scene object type." << std::endl;

        }
    }
}
void DefaultSimulator::run()
{
    beginSim();

    for ( size_t i = 0; i < this->objectsSimulated.size(); i++ )
    {
        auto sceneObj = this->objectsSimulated[i];

        //ensure that dummy simulator will work on static scene objects only.
        if ( sceneObj->getType() == core::ClassType::StaticSceneObject )
        {
            auto sceneObject = std::static_pointer_cast<StaticSceneObject>(sceneObj);

            for(auto apply : this->operatorFunctions)
            {
                apply(sceneObject->getLocalVertices());
            }
            this->updateHapticForces(sceneObject);
        }
    }

    endSim();
}
void DefaultSimulator::endSim()
{
    //end the job
}
void DefaultSimulator::syncBuffers()
{
    for ( size_t i = 0; i < this->objectsSimulated.size(); i++ )
    {
        auto sceneObj = this->objectsSimulated[i];

        //ensure that dummy simulator will work on static scene objects only.
        if ( sceneObj->getType() == core::ClassType::StaticSceneObject )
        {
            auto sceneObject = std::static_pointer_cast<StaticSceneObject>(sceneObj);
            auto model = std::dynamic_pointer_cast<MeshCollisionModel>(sceneObject->getModel());
            if(!model)
            {
                std::cerr << "Unknown model type." << std::endl;
                break;
            }
            auto mesh = std::static_pointer_cast<MeshCollisionModel>(model)->getMesh();
            // WARNING: Copy??!
            mesh->getVertices() = sceneObject->getLocalVertices();
        }
    }
}
void DefaultSimulator::handleEvent(std::shared_ptr<core::Event> p_event )
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

void DefaultSimulator::updateHapticForces(std::shared_ptr<StaticSceneObject> sceneObject)
{
    auto outputDevice = std::dynamic_pointer_cast<VRPNForceDevice>(this->hapticTool->getOutputDevice());
    if(!outputDevice)
    {
        core::Vec3f normal(0,1,0);
        outputDevice->setContactPlane(normal,100);
        return;
    }

    auto forces = sceneObject->getContactForces();
    auto points = sceneObject->getContactPoints();

    if(forces.size() == 0)
    {
        core::Vec3f normal(0,1,0);
        outputDevice->setContactPlane(normal,100);
        return;
    }

    core::Vec3d totalForce = core::Vec3d::Zero();
    for(const auto &f : forces)
    {
        totalForce = f.second;
    }
    core::Vec3d contactPoint = core::Vec3d::Zero();

    for(const auto &p : points)
    {
        contactPoint = p.second;
    }
    contactPoint /= points.size();

    float norm = totalForce.norm();
    auto normal = totalForce.normalized();
    auto d = totalForce.dot(contactPoint);

    outputDevice->setContactPlane(normal.cast<float>(),0);
    outputDevice->setDampingCoefficient(0.001);
    outputDevice->setDynamicFriction(0.0);
    outputDevice->setSpringCoefficient(norm);
    outputDevice->setStaticFriction(0.0);

}
