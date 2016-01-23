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

// iMSTK includes
#include "Simulators/DefaultSimulator.h"
#include "SceneModels/StaticSceneObject.h"
#include "Core/Event.h"
#include "Event/KeyboardEvent.h"
#include "Collision/MeshCollisionModel.h"
#include "Devices/VRPNForceDevice.h"
#include "VirtualTools/ToolCoupler.h"

DefaultSimulator::DefaultSimulator(  ) : ObjectSimulator( )
{
    this->addOperation([](std::shared_ptr<MeshModel> &model)
    {
        for(auto &x : model->getMesh()->getVertices())
        {
            x.array() += 0.000001;
        }
    });
}
void DefaultSimulator::beginExecution()
{
    //start the job
}
void DefaultSimulator::initialize()
{
    //do nothing for now
}
void DefaultSimulator::run()
{
    beginExecution();

    for(auto &model : this->simulatedModels)
    {
        model->update(this->timeStep);
        this->updateHapticForces(model);
    }

    endExecution();
}
void DefaultSimulator::endExecution()
{
    //end the job
}
void DefaultSimulator::syncBuffers()
{

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

void DefaultSimulator::updateHapticForces(std::shared_ptr<SceneObject> sceneObject)
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
//     auto d = totalForce.dot(contactPoint);

    outputDevice->setContactPlane(normal.cast<float>(),0);
    outputDevice->setDampingCoefficient(0.001);
    outputDevice->setDynamicFriction(0.0);
    outputDevice->setSpringCoefficient(norm);
    outputDevice->setStaticFriction(0.0);
}
