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

#include "Simulators/VegaFEMModelSimulator.h"

// SimMedTK includes
#include "Devices/VRPNForceDevice.h"
#include "Simulators/VegaFemSceneObject.h"
#include "VirtualTools/ToolCoupler.h"

VegaFEMModelSimulator::VegaFEMModelSimulator( ) : ObjectSimulator(  )
{
    hapticButtonPressed = false;
}

//---------------------------------------------------------------------------
void VegaFEMModelSimulator::initialize()
{//do nothing for now
    for ( size_t i = 0; i < simulatedModels.size(); i++ )
    {
        auto object = simulatedModels[i];

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

//---------------------------------------------------------------------------
void VegaFEMModelSimulator::handleEvent(std::shared_ptr<core::Event> /*p_event*/ )
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

//---------------------------------------------------------------------------
void VegaFEMModelSimulator::setHapticTool(std::shared_ptr< ToolCoupler > tool)
{
    this->hapticTool = tool;
}

//---------------------------------------------------------------------------
std::shared_ptr< ToolCoupler > VegaFEMModelSimulator::getHapticTool() const
{
    return this->hapticTool;
}

//---------------------------------------------------------------------------
void VegaFEMModelSimulator::updateHapticForces(std::shared_ptr<VegaFemSceneObject> sceneObject)
{
    if(!this->hapticTool)
    {
        return;
    }

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
        totalForce += f.second;
    }
    core::Vec3d contactPoint = core::Vec3d::Zero();

    for(const auto &p : points)
    {
        contactPoint += p.second;
    }
    contactPoint /= points.size();

    float norm = totalForce.norm();
    auto normal = totalForce.normalized();
    auto d = normal.dot(contactPoint);

    outputDevice->setContactPlane(normal.cast<float>(),d);
    outputDevice->setDampingCoefficient(0.001);
    outputDevice->setDynamicFriction(0.0);
    outputDevice->setSpringCoefficient(norm);
    outputDevice->setStaticFriction(0.0);
}
