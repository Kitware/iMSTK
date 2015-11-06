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
    vtkCamera* camera)
{
    this->inputDevice = inputDevice;
    this->poolDelay = std::chrono::milliseconds(100);
    this->initialTransform.setIdentity();
    this->name = "LaparoscopicCameraCoupler";

    initializeCameraScopeConfiguration();

    this->camera = camera;

    cameraPosOrientData = std::make_shared<cameraConfigurationData>();
}

LaparoscopicCameraCoupler::LaparoscopicCameraCoupler(
    std::shared_ptr<DeviceInterface> inputDevice)
{
    this->inputDevice = inputDevice;
    this->poolDelay = std::chrono::milliseconds(100);
    this->initialTransform.Identity();
    this->name = "LaparoscopicCameraCoupler";

    initializeCameraScopeConfiguration();

    cameraPosOrientData = std::make_shared<cameraConfigurationData>();
}

LaparoscopicCameraCoupler::~LaparoscopicCameraCoupler()
{
}

void LaparoscopicCameraCoupler::initializeCameraScopeConfiguration()
{
    this->bendingRadius = 1.0; // default bending radius
    angleX = 0.0;
    angleY = 0.0;
    maxAngleX = maxAngleY = 11.0 / 7; //+ 90 deg
    minAngleX = minAngleY = -11.0 / 7; //- 90 deg
    deltaAngleXY = (4.0 / 360)*(22.0 / 7);// 2 deg
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

void LaparoscopicCameraCoupler::init()
{
    this->orientation.setIdentity();
    this->position.setZero();

    // Open communication for the device
    this->inputDevice->openDevice();
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
        if (!this->updateCamera())
        {
            this->terminate();
        }

        if (this->inputDevice->getButton(0))
        {
            this->screenCaptureData->triggerScreenCapture = true;
        }

        std::this_thread::sleep_for(poolDelay);
    }
    this->inputDevice->closeDevice();

    this->terminate();
}

bool LaparoscopicCameraCoupler::updateCamera()
{
    if (!this->inputDevice)
    {
        std::cout << "Invalid input device" << std::endl;
        return false;
    }

    if (this->inputDevice->getButton(0) && angleY < maxAngleY)
    {
        angleY += deltaAngleXY;
    }

    if (this->inputDevice->getButton(1) && angleY > minAngleY)
    {
        angleY -= deltaAngleXY;
    }

    if (this->inputDevice->getButton(3) && angleX < maxAngleX)
    {
        angleX += deltaAngleXY;
    }

    if (this->inputDevice->getButton(4) && angleX > minAngleX)
    {
        angleX -= deltaAngleXY;
    }

    core::Quaterniond newDeviceRot = inputDevice->getOrientation();
    core::Vec3d newDevicePos = inputDevice->getPosition()*this->scalingFactor;

    core::Vec3d bendingOffset = core::Vec3d(0, 0, this->bendingRadius);

    core::Quaterniond bendingRot(cos(angleY/2), 0, sin(angleY/2), 0);
    bendingRot.normalize();
    bendingRot = bendingRot*core::Quaterniond(cos(angleX/2), sin(angleX/2), 0, 0);
    bendingRot.normalize();

    // update the camera position, focus and up vector data
    cameraPosOrientData->focus = newDeviceRot*bendingRot*core::Vec3d(0, 0, -200.0);
    cameraPosOrientData->upVector = newDeviceRot*bendingRot*core::Vec3d(0, 1.0, 0);

    cameraPosOrientData->position =
        newDeviceRot*(bendingOffset - bendingRot*bendingOffset) + newDevicePos;

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

std::shared_ptr<cameraConfigurationData> LaparoscopicCameraCoupler::getCameraData()
{
    return cameraPosOrientData;
};

double LaparoscopicCameraCoupler::getBendingRadius() const
{
    return bendingRadius;
}

void LaparoscopicCameraCoupler::setBendingRadius(const double val)
{
    bendingRadius = val;
}

std::shared_ptr<screenShotData> LaparoscopicCameraCoupler::getScreenCaptureData()
{
    return screenCaptureData;
}

void LaparoscopicCameraCoupler::enableScreenCapture()
{
    this->screenCaptureData = std::make_shared<screenShotData>();
}