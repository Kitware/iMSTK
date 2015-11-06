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

#ifndef iSMTK_LAPAROSCOPIC_CAMERA_COUPLER_H
#define iSMTK_LAPAROSCOPIC_CAMERA_COUPLER_H

#include <memory>
#include <chrono>

// iMSTK includes
#include "Core/Module.h"
#include "Core/Quaternion.h"
#include "Core/Vector.h"

// vtk includes
#include <vtkCamera.h>
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>
#include "vtkNew.h"

class DeviceInterface;

struct cameraConfigurationData
{
    core::Vec3d position;
    core::Vec3d focus;
    core::Vec3d upVector;

    cameraConfigurationData() : position(0, 0, 0), focus(0, 0, -1), upVector(0, 1, 0){};
    ~cameraConfigurationData(){};
};

struct screenShotData
{
    vtkNew<vtkWindowToImageFilter> windowToImageFilter;
    vtkNew<vtkPNGWriter> pngWriter;
    bool triggerScreenCapture;
    int screenShotNumber;

    screenShotData() : triggerScreenCapture(false), screenShotNumber(0)
    {
        windowToImageFilter->SetMagnification(1);

        windowToImageFilter->SetInputBufferTypeToRGB();

        windowToImageFilter->ReadFrontBufferOff();

        windowToImageFilter->Update();

        pngWriter->SetInputConnection(windowToImageFilter->GetOutputPort());
    };

    ~screenShotData(){};
};

class LaparoscopicCameraCoupler : public Module
{
public:
	using TransformType = Eigen::Transform<double, 3, Eigen::Isometry>;

public:
    ///
    /// \brief Constructor
    ///
    LaparoscopicCameraCoupler() = default;

    LaparoscopicCameraCoupler(
        std::shared_ptr<DeviceInterface> inputDevice,
        vtkCamera* camera);

    LaparoscopicCameraCoupler(std::shared_ptr<DeviceInterface> inputDevice);

	///
	/// \brief Destructor
	///
    ~LaparoscopicCameraCoupler();

	///
	/// \brief Initialize the camera scope configuration
	///
    void initializeCameraScopeConfiguration();

    ///
    /// \brief Set the input device for this tool
    /// \param newDevice A pointer to an allocated device
    ///
    void setInputDevice(std::shared_ptr<DeviceInterface> newDevice);

    ///
    /// \brief Set the output device for this tool coupler
    ///
    std::shared_ptr<DeviceInterface> getInputDevice();

    ///
    /// \brief Set the pointer to the mesh to control
    /// \param newMesh A pointer to an allocated mesh
    ///
    void setCamera(vtkCamera* newCamera);

    ///
    /// \brief Get the output device for this tool coupler
    ///
    vtkCamera* getcamera() const;

    ///
    /// \brief Get the current pooling delay
    /// \return The currently set polling delay
    ///
    const std::chrono::milliseconds &getPoolDelay() const;

    ///
    /// \brief Set the pooling delay of the controller to get new data from the device
    /// \param delay The new polling delay to set
    ///
    void setPoolDelay(const std::chrono::milliseconds &delay);

    ///
    /// \brief Get the current scaling factor
    /// \return The currently set scaling factor
    ///
    const double &getScalingFactor() const;

    ///
    /// \brief Set how much to scale the physical movement by in 3D space
    /// \param The new scaling factor to set
    ///
    void setScalingFactor(const double factor);

    ///
    /// \brief Get the current orientation
    /// \return The orientation
    ///
    const core::Quaterniond &getOrientation() const;

    ///
    /// \brief Set orientation much to scale the physical movement by in 3D space
    /// \param  The new scaling factor to set
    ///
    void setOrientation(const Eigen::Map<core::Quaterniond> &newOrientation);

    ///
    /// \brief Get the current orientation
    /// \return The currently set scaling factor
    ///
    const core::Vec3d &getPosition() const;

    ///
    /// \brief Set orientation much to scale the physical movement by in 3D space
    /// \param  The new scaling factor to set
    ///
    void setPosition(const core::Vec3d &newPosition);

    ///
    /// \brief Set offset orientation much to scale the physical movement by in 3D space
    /// \param  The new scaling factor to set
    ///
    void setOffsetOrientation(const Eigen::Map<core::Quaterniond> &offsetOrientation);

    ///
    /// \brief Get the offset orientation
    /// \return The orientation
    ///
    const core::Quaterniond &getOffsetOrientation() const;

    ///
    /// \brief Get the offset position
    /// \return The currently set scaling factor
    ///
    const core::Vec3d &getOffsetPosition() const;

    ///
    /// \brief Set offset orientation much to scale the physical movement by in 3D space
    /// \param  The new scaling factor to set
    ///
    void setOffsetPosition(const core::Vec3d &offsetPosition);

    ///
    /// \brief Update position and orientation of the camera based on device data
    ///
    bool updateCamera();

    ///
	/// \brief Returns the pointer to the camera data that is updated by this controller
	///
    std::shared_ptr<cameraConfigurationData> getCameraData();

    ///
    /// \brief Getter and setter for the bending radius of the tip of the scope
    ///
    double getBendingRadius() const;
    void setBendingRadius(const double val);

	///
	/// \brief Get the screen capture related data
	///
    std::shared_ptr<screenShotData> getScreenCaptureData();

	///
	/// \brief Initializes screen capture capability
	///
    void enableScreenCapture();

    ///
    /// \brief Module overrides
    ///
    bool init() override;
    void beginFrame() override;
    void endFrame() override;

    ///
    /// \brief Update tracker and
    ///
    void exec() override;

private:
    TransformType initialTransform; //!< Transform applied to the position obtained from device
    core::Quaterniond orientation; //!< Previous rotation quaternion from device

    core::Quaterniond prevOrientation; //!< Previous rotation quaternion from phantom
    core::Vec3d prevPosition;          //!< Previous position from phantom

    core::Vec3d position;          //!< Previous position from device
    double scalingFactor;          //!< Scaling factor for physical to virtual translation

    core::Quaterniond offsetOrientation; //!< Previous rotation quaternion from device
    core::Vec3d offsetPosition;          //!< Previous position from device

    std::chrono::milliseconds poolDelay;  //!< Pooling delay
    vtkCamera* camera; //!< Pointer to rendering camera
    std::shared_ptr<DeviceInterface> inputDevice;  //!< Pointer to input device

    double bendingRadius;//!< bending radius of the tip of the scope

    double angleY;
    double maxAngleY;
    double minAngleY;

    double angleX;
    double maxAngleX;
    double minAngleX;
    double deltaAngleXY;

    std::shared_ptr<cameraConfigurationData> cameraPosOrientData;//!< camera config data

    std::shared_ptr<screenShotData> screenCaptureData;

    std::shared_ptr<vtkWindowToImageFilter> windowToImageFilter;
};

#endif // iSMTK_LAPAROSCOPIC_CAMERA_COUPLER_H
