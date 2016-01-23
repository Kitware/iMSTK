// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
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
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#include "LaparoscopicCameraController.h"

#include <thread>

#include "Core/BaseMesh.h"
#include "Devices/DeviceInterface.h"
#include "Core/RenderDelegate.h"

LaparoscopicCameraController::LaparoscopicCameraController(
    std::shared_ptr< DeviceInterface > inputDevice, vtkCamera* camera)
{
    this->name = "LaparoscopicCameraController";
    this->inputDevice = inputDevice;
    this->camera = camera;

    initializeCameraScopeConfiguration();
}

LaparoscopicCameraController::LaparoscopicCameraController(
    std::shared_ptr<DeviceInterface> inputDevice)
{
    this->name = "LaparoscopicCameraController";
    this->inputDevice = inputDevice;

    initializeCameraScopeConfiguration();
}

LaparoscopicCameraController::~LaparoscopicCameraController()
{
}

void LaparoscopicCameraController::initializeCameraScopeConfiguration()
{
    this->poolDelay = std::chrono::milliseconds(100);
    this->bendingRadius = 1.0;
    this->scalingFactor = 1.0;
    this->currentAngleX = 0.0;
    this->currentAngleY = 0.0;
    this->maxAngleX = this->maxAngleY = 11.0 / 7; // 90 deg
    this->minAngleX = this->minAngleY = -11.0 / 7; //- 90 deg
    this->deltaAngleXY = (4.0 / 360)*(22.0 / 7);// 2 deg
}

void LaparoscopicCameraController::setInputDevice(
    std::shared_ptr<DeviceInterface> newDevice)
{
    this->inputDevice = newDevice;
}

std::shared_ptr< DeviceInterface > LaparoscopicCameraController::getInputDevice()
{
    return this->inputDevice;
}

void LaparoscopicCameraController::setCamera(vtkCamera* newCamera)
{
    this->camera = newCamera;
}

vtkCamera* LaparoscopicCameraController::getcamera() const
{
    return this->camera;
}

void LaparoscopicCameraController::setPoolDelay(const std::chrono::milliseconds& delay)
{
    this->poolDelay = delay;
}

const std::chrono::milliseconds &LaparoscopicCameraController::getPoolDelay() const
{
    return this->poolDelay;
}

void LaparoscopicCameraController::setBendingRadius(const double val)
{
    this->bendingRadius = val;
}

double LaparoscopicCameraController::getBendingRadius() const
{
    return this->bendingRadius;
}

void LaparoscopicCameraController::setScalingFactor(const double factor)
{
    this->scalingFactor = factor;
}

const double& LaparoscopicCameraController::getScalingFactor() const
{
    return this->scalingFactor;
}

bool LaparoscopicCameraController::init()
{
    // Open communication for the device
    this->inputDevice->openDevice();
    return true;
}

void LaparoscopicCameraController::beginFrame()
{
}

void LaparoscopicCameraController::endFrame()
{
}

void LaparoscopicCameraController::exec()
{
    if(!this->camera)
    {
        this->terminate();
    }

    while(!this->terminateExecution)
    {
        if (!this->updateCamera())
        {
            this->terminate();
        }

        std::this_thread::sleep_for(this->poolDelay);
    }

    // Ensure proper shutdown takes place for the inputDevice
    //  -Thread MUST be terminated before calling closeDevice()
    //    -This is to prevent mainloops that do not check for deleted objects
    //     from running.
    this->inputDevice->terminate();
    this->inputDevice->waitTermination();
    this->inputDevice->closeDevice();

    this->terminationCompleted = true;
}

bool LaparoscopicCameraController::updateCamera()
{
    if (!this->inputDevice)
    {
        std::cerr << "updateCamera: Invalid input device" << std::endl;
        return false;
    }
    if (!this->camera)
    {
        std::cerr << "updateCamera: Missing camera" << std::endl;
        return false;
    }

    if (this->inputDevice->getButton(3) && this->currentAngleY < this->maxAngleY)
    {
        this->currentAngleY += this->deltaAngleXY;
    }
    if (this->inputDevice->getButton(4) && this->currentAngleY > this->minAngleY)
    {
        this->currentAngleY -= this->deltaAngleXY;
    }
    if (this->inputDevice->getButton(0) && this->currentAngleX < this->maxAngleX)
    {
        this->currentAngleX += this->deltaAngleXY;
    }
    if (this->inputDevice->getButton(1) && this->currentAngleX > this->minAngleX)
    {
        this->currentAngleX -= this->deltaAngleXY;
    }
    core::Quaterniond newDeviceRot = this->inputDevice->getOrientation();
    core::Vec3d newDevicePos = this->inputDevice->getPosition()*this->scalingFactor;
    core::Vec3d bendingOffset = core::Vec3d(0, 0, this->bendingRadius);
    core::Quaterniond bendingRot(cos(this->currentAngleY/2), 0, sin(this->currentAngleY/2), 0);
    bendingRot.normalize();
    bendingRot = bendingRot*core::Quaterniond(cos(this->currentAngleX/2), sin(this->currentAngleX/2), 0, 0);
    bendingRot.normalize();

    // Update the camera position, focus and up vector data
    core::Vec3d position = newDeviceRot*(bendingOffset - bendingRot*bendingOffset) + newDevicePos;
    core::Vec3d upVector = newDeviceRot*bendingRot*core::Vec3d(0, 1.0, 0);
    core::Vec3d focus = newDeviceRot*bendingRot*core::Vec3d(0, 0, -200.0);

    this->camera->SetPosition(position[0], position[1], position[2]);
    this->camera->SetViewUp(upVector[0], upVector[1], upVector[2]);
    this->camera->SetFocalPoint(focus[0], focus[1], focus[2]);

    return true;
}
