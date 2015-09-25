// This file is part of the SimMedTK project.
// Copyright (c) Kitware, Inc.
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

#include "ToolCoupler.h"

#include <thread>

#include "Core/BaseMesh.h"
#include "Devices/DeviceInterface.h"
#include "Core/RenderDelegate.h"

//---------------------------------------------------------------------------
ToolCoupler::ToolCoupler(std::shared_ptr< DeviceInterface > inputDevice,
                         std::shared_ptr< Core::BaseMesh > toolMesh)
{
    this->inputDevice = inputDevice;
    this->outputDevice = inputDevice;
    this->pollDelay = std::chrono::milliseconds(100);
    this->mesh = toolMesh;
    this->initialTransform.setIdentity();
    this->name = "ToolCoupler";
}

//---------------------------------------------------------------------------
ToolCoupler::ToolCoupler(std::shared_ptr<DeviceInterface> inputDevice)
{
    this->inputDevice = inputDevice;
    this->outputDevice = inputDevice;
    this->pollDelay = std::chrono::milliseconds(100);
    this->initialTransform.Identity();
    this->name = "ToolCoupler";
}

//---------------------------------------------------------------------------
ToolCoupler::ToolCoupler(std::shared_ptr< DeviceInterface > inputDevice,
                         std::shared_ptr< DeviceInterface > outputDevice,
                         std::shared_ptr< Core::BaseMesh > toolMesh)
{
    this->inputDevice = inputDevice;
    this->outputDevice = outputDevice;
    this->pollDelay = std::chrono::milliseconds(100);
    this->mesh = toolMesh;
    this->initialTransform.Identity();
    this->name = "ToolCoupler";
}

//---------------------------------------------------------------------------
ToolCoupler::~ToolCoupler()
{
}

//---------------------------------------------------------------------------
void ToolCoupler::setInputDevice(std::shared_ptr< DeviceInterface > newDevice)
{
    this->inputDevice = newDevice;
}

//---------------------------------------------------------------------------
std::shared_ptr< DeviceInterface > ToolCoupler::getInputDevice() const
{
    return this->inputDevice;
}

//---------------------------------------------------------------------------
void ToolCoupler::setOutpurDevice(std::shared_ptr< DeviceInterface > newDevice)
{
    this->outputDevice = newDevice;
}

//---------------------------------------------------------------------------
std::shared_ptr< DeviceInterface > ToolCoupler::getOutpurDevice() const
{
    return this->outputDevice;
}

//---------------------------------------------------------------------------
void ToolCoupler::setMesh(std::shared_ptr< Core::BaseMesh > newMesh)
{
    this->mesh = newMesh;
}

//---------------------------------------------------------------------------
std::shared_ptr< Core::BaseMesh > ToolCoupler::getMesh() const
{
    return this->mesh;
}

//---------------------------------------------------------------------------
const std::chrono::milliseconds &ToolCoupler::getPollDelay() const
{
    return this->pollDelay;
}

//---------------------------------------------------------------------------
void ToolCoupler::setPollDelay(const std::chrono::milliseconds& delay)
{
    this->pollDelay = delay;
}

//---------------------------------------------------------------------------
const double& ToolCoupler::getScalingFactor() const
{
    return this->scalingFactor;
}

//---------------------------------------------------------------------------
void ToolCoupler::setScalingFactor(const double& factor)
{
    this->scalingFactor = factor;
}

//---------------------------------------------------------------------------
const Eigen::Quaternion<double> &ToolCoupler::getOrientation() const
{
    return this->orientation;
}

//---------------------------------------------------------------------------
void ToolCoupler::setOrientation(const Eigen::Map< Eigen::Quaternion< double > >& newOrientation)
{
    this->orientation = newOrientation;
}

//---------------------------------------------------------------------------
void ToolCoupler::init()
{
    if(!this->outputDevice)
    {
        this->outputDevice = this->inputDevice;
    }

    this->orientation.setIdentity();
    this->position.setZero();

    // Open communication for the device
    this->inputDevice->openDevice();
}

//---------------------------------------------------------------------------
void ToolCoupler::beginFrame() {}

//---------------------------------------------------------------------------
void ToolCoupler::endFrame() {}

//---------------------------------------------------------------------------
void ToolCoupler::exec()
{
    if(!this->mesh)
    {
        this->terminate();
    }

    while(!this->terminateExecution)
    {
        if(!this->updateTracker() || !this->updateForces())
        {
            this->terminate();
        }
        std::this_thread::sleep_for(pollDelay);
    }
    this->terminationCompleted = true;
    this->inputDevice->closeDevice();
}

//---------------------------------------------------------------------------
bool ToolCoupler::updateTracker()
{
    if(!this->inputDevice)
    {
        std::cout << "Invalid input device" << std::endl;
        return false;
    }
    core::Quaterniond rot = inputDevice->getOrientation();
    core::Vec3d pos = inputDevice->getPosition() * this->scalingFactor;

    Eigen::Quaterniond rotation(rot * this->orientation);
    Eigen::Translation3d translation(pos - rotation*this->position);

    this->mesh->transform(/*this->initialTransform**/translation*rotation);

    this->mesh->getRenderDelegate()->modified(); //tell the renderer to update

    this->position = pos;
    this->orientation = rot.conjugate();

    return true;
}

//---------------------------------------------------------------------------
bool ToolCoupler::updateForces()
{
    if(!this->outputDevice)
    {
        std::cout << "Invalid output device" << std::endl;
        return false;
    }

    return true;
}
