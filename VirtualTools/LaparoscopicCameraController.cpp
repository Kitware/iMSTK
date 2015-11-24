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

#include "LaparoscopicCameraController.h"

#include <thread>

#include "Core/BaseMesh.h"
#include "Devices/DeviceInterface.h"
#include "Core/RenderDelegate.h"

LaparoscopicCameraController::LaparoscopicCameraController(
    std::shared_ptr< DeviceInterface > inputDevice,
    vtkCamera* camera)
{
    this->inputDevice = inputDevice;
    this->poolDelay = std::chrono::milliseconds(100);
    this->initialTransform.setIdentity();
    this->name = "LaparoscopicCameraCoupler";

    initializeCameraScopeConfiguration();

    this->camera = camera;

    this->cameraPosOrientData = std::make_shared<cameraConfigurationData>();
}

LaparoscopicCameraController::LaparoscopicCameraController(
    std::shared_ptr<DeviceInterface> inputDevice)
{
    this->inputDevice = inputDevice;
    this->poolDelay = std::chrono::milliseconds(100);
    this->initialTransform.Identity();
    this->name = "LaparoscopicCameraCoupler";

    initializeCameraScopeConfiguration();

    this->cameraPosOrientData = std::make_shared<cameraConfigurationData>();
}

LaparoscopicCameraController::~LaparoscopicCameraController()
{
}

void LaparoscopicCameraController::initializeCameraScopeConfiguration()
{
    this->bendingRadius = 1.0; // default bending radius
    this->angleX = 0.0;
    angleY = 0.0;
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

const std::chrono::milliseconds &LaparoscopicCameraController::getPoolDelay() const
{
    return this->poolDelay;
}

void LaparoscopicCameraController::setPoolDelay(const std::chrono::milliseconds& delay)
{
    this->poolDelay = delay;
}

const double& LaparoscopicCameraController::getScalingFactor() const
{
    return this->scalingFactor;
}

void LaparoscopicCameraController::setScalingFactor(const double factor)
{
    this->scalingFactor = factor;
}

const Eigen::Quaternion<double> &LaparoscopicCameraController::getOrientation() const
{
    return this->orientation;
}

void LaparoscopicCameraController::setOrientation(
    const Eigen::Map<Eigen::Quaternion<double>>& newOrientation)
{
    this->orientation = newOrientation;
}

void LaparoscopicCameraController::init()
{
    this->orientation.setIdentity();
    this->position.setZero();

    // Open communication for the device
    this->inputDevice->openDevice();
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
    this->inputDevice->closeDevice();

    this->terminate();
}

bool LaparoscopicCameraController::updateCamera()
{
    if (!this->inputDevice)
    {
        std::cout << "Invalid input device" << std::endl;
        return false;
    }

    if (this->inputDevice->getButton(3) && this->angleY < this->maxAngleY)
    {
        this->angleY += this->deltaAngleXY;
    }

    if (this->inputDevice->getButton(4) && this->angleY > this->minAngleY)
    {
        this->angleY -= this->deltaAngleXY;
    }

    if (this->inputDevice->getButton(0) && this->angleX < this->maxAngleX)
    {
        this->angleX += this->deltaAngleXY;
    }

    if (this->inputDevice->getButton(1) && this->angleX > this->minAngleX)
    {
        this->angleX -= this->deltaAngleXY;
    }

    core::Quaterniond newDeviceRot = this->inputDevice->getOrientation();
    core::Vec3d newDevicePos = this->inputDevice->getPosition()*this->scalingFactor;

    core::Vec3d bendingOffset = core::Vec3d(0, 0, this->bendingRadius);

    core::Quaterniond bendingRot(cos(this->angleY/2), 0, sin(this->angleY/2), 0);
    bendingRot.normalize();
    bendingRot = bendingRot*core::Quaterniond(cos(this->angleX/2), sin(this->angleX/2), 0, 0);
    bendingRot.normalize();

    // update the camera position, focus and up vector data
    this->cameraPosOrientData->focus = newDeviceRot*bendingRot*core::Vec3d(0, 0, -200.0);
    this->cameraPosOrientData->upVector = newDeviceRot*bendingRot*core::Vec3d(0, 1.0, 0);

    this->cameraPosOrientData->position =
        newDeviceRot*(bendingOffset - bendingRot*bendingOffset) + newDevicePos;

    return true;
}

void LaparoscopicCameraController::setOffsetOrientation(
    const Eigen::Map<core::Quaterniond> &offsetOrientation)
{
    this->offsetOrientation = offsetOrientation;
}

void LaparoscopicCameraController::setOffsetPosition(const core::Vec3d &offsetPosition)
{
    this->offsetPosition = offsetPosition;
}

const core::Quaterniond & LaparoscopicCameraController::getOffsetOrientation() const
{
    return this->offsetOrientation;
}

const core::Vec3d & LaparoscopicCameraController::getOffsetPosition() const
{
    return this->offsetPosition;
}

std::shared_ptr<cameraConfigurationData> LaparoscopicCameraController::getCameraData()
{
    return this->cameraPosOrientData;
};

double LaparoscopicCameraController::getBendingRadius() const
{
    return this->bendingRadius;
}

void LaparoscopicCameraController::setBendingRadius(const double val)
{
    this->bendingRadius = val;
}
