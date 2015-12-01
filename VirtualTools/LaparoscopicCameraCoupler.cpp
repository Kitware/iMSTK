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

#include "LaparoscopicCameraCoupler.h"

#include <thread>

#include "Core/BaseMesh.h"
#include "Devices/DeviceInterface.h"
#include "Core/RenderDelegate.h"

LaparoscopicCameraCoupler::LaparoscopicCameraCoupler(
    std::shared_ptr< DeviceInterface > inputDevice,
    std::shared_ptr< vtkCamera > camera)
{
    this->inputDevice = inputDevice;
    this->poolDelay = std::chrono::milliseconds(100);
    this->initialTransform.setIdentity();
    this->name = "LaparoscopicCameraCoupler";
}

LaparoscopicCameraCoupler::LaparoscopicCameraCoupler(
    std::shared_ptr<DeviceInterface> inputDevice)
{
    this->inputDevice = inputDevice;
    this->poolDelay = std::chrono::milliseconds(100);
    this->initialTransform.Identity();
    this->name = "LaparoscopicCameraCoupler";
}

LaparoscopicCameraCoupler::~LaparoscopicCameraCoupler()
{
}

void LaparoscopicCameraCoupler::setInputDevice(std::shared_ptr<DeviceInterface> newDevice)
{
    this->inputDevice = newDevice;
}

std::shared_ptr< DeviceInterface > LaparoscopicCameraCoupler::getInputDevice()
{
    return this->inputDevice;
}

void LaparoscopicCameraCoupler::setCamera(vtkCamera* newCamera)
{
    this->camera = newCamera;
}

vtkCamera* LaparoscopicCameraCoupler::getcamera() const
{
    return this->camera;
}

const std::chrono::milliseconds &LaparoscopicCameraCoupler::getPoolDelay() const
{
    return this->poolDelay;
}

void LaparoscopicCameraCoupler::setPoolDelay(const std::chrono::milliseconds& delay)
{
    this->poolDelay = delay;
}

const double& LaparoscopicCameraCoupler::getScalingFactor() const
{
    return this->scalingFactor;
}

void LaparoscopicCameraCoupler::setScalingFactor(const double factor)
{
    this->scalingFactor = factor;
}

const Eigen::Quaternion<double> &LaparoscopicCameraCoupler::getOrientation() const
{
    return this->orientation;
}

void LaparoscopicCameraCoupler::setOrientation(
    const Eigen::Map<Eigen::Quaternion<double>>& newOrientation)
{
    this->orientation = newOrientation;
}

bool LaparoscopicCameraCoupler::init()
{
    this->orientation.setIdentity();
    this->position.setZero();

    // Open communication for the device
    this->inputDevice->openDevice();
    return true;
}

void LaparoscopicCameraCoupler::beginFrame()
{
}

void LaparoscopicCameraCoupler::endFrame()
{
}

void LaparoscopicCameraCoupler::exec()
{
    if(!this->camera)
    {
        this->terminate();
    }

    while(!this->terminateExecution)
    {
        if(!this->updateTracker())
        {
            this->terminate();
        }
        std::this_thread::sleep_for(poolDelay);
    }
    this->terminationCompleted = true;
    this->inputDevice->closeDevice();
}

bool LaparoscopicCameraCoupler::updateTracker()
{
    if (!this->inputDevice)
    {
        std::cout << "Invalid input device" << std::endl;
        return false;
    }

    core::Quaterniond newRot = inputDevice->getOrientation();
    core::Vec3d newPos = inputDevice->getPosition() * this->scalingFactor;

    core::Vec3d transformedFocus = newRot._transformVector(core::Vec3d(0, 0, -200));
    core::Vec3d transformedUpVector = newRot._transformVector(core::Vec3d(0, 1, 0));

//     std::cout << "newPos" << newPos << std::endl;
//     std::cout << "transformedUpVector" << transformedUpVector << std::endl;
//     std::cout << "transformedFocus" << transformedFocus << std::endl;
    this->camera->SetPosition(newPos[0], newPos[1], newPos[2]);

    this->camera->SetViewUp(
        transformedUpVector[0],
        transformedUpVector[1],
        transformedUpVector[2]);

    this->camera->SetFocalPoint(
        transformedFocus[0],
        transformedFocus[1],
        transformedFocus[2]);

    return true;
}

void LaparoscopicCameraCoupler::setOffsetOrientation(
    const Eigen::Map<core::Quaterniond> &offsetOrientation)
{
    this->offsetOrientation = offsetOrientation;
}

void LaparoscopicCameraCoupler::setOffsetPosition(const core::Vec3d &offsetPosition)
{
    this->offsetPosition = offsetPosition;
}

const core::Quaterniond & LaparoscopicCameraCoupler::getOffsetOrientation() const
{
    return this->offsetOrientation;
}

const core::Vec3d & LaparoscopicCameraCoupler::getOffsetPosition() const
{
    return this->offsetPosition;
}
