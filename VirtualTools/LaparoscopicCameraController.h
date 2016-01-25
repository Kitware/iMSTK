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

#ifndef VIRTUAL_TOOLS_LAPAROSCOPICCAMERACONTROLLER_H
#define VIRTUAL_TOOLS_LAPAROSCOPICCAMERACONTROLLER_H

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

namespace imstk {

class DeviceInterface;

///
/// \class LaparoscopicCameraController
///
/// \brief This class enables the controll of the vtk camera using an external device
/// Additionally it allows for tip deflection just like you would see in some laparoscopic
/// camera scopes.
///
class LaparoscopicCameraController : public Module
{
public:
	using TransformType = Eigen::Transform<double, 3, Eigen::Isometry>;

public:
    ///
    /// \brief Constructor
    ///
    LaparoscopicCameraController() = default;

    LaparoscopicCameraController(
        std::shared_ptr<DeviceInterface> inputDevice,
        vtkCamera* camera);

    LaparoscopicCameraController(std::shared_ptr<DeviceInterface> inputDevice);

	///
	/// \brief Destructor
	///
    ~LaparoscopicCameraController();

	///
	/// \brief Initialize the camera scope configuration
	///
    void initializeCameraScopeConfiguration();

    ///
    /// \brief Set/Get the input device for this tool coupler
    ///
    void setInputDevice(std::shared_ptr<DeviceInterface> newDevice);
    std::shared_ptr<DeviceInterface> getInputDevice();

    ///
    /// \brief Set/Get the pointer to the controlled camera
    ///
    void setCamera(vtkCamera* newCamera);
    vtkCamera* getcamera() const;

    ///
    /// \brief Set/Get the pooling delay of the controller
    ///
    void setPoolDelay(const std::chrono::milliseconds& delay);
    const std::chrono::milliseconds &getPoolDelay() const;

    ///
    /// \brief Set/Get the bending radius of the tip of the scope
    ///
    void setBendingRadius(const double val);
    double getBendingRadius() const;

    ///
    /// \brief Set/Get the current scaling factor
    ///
    void setScalingFactor(const double factor);
    const double &getScalingFactor() const;

    ///
    /// \brief Update position and orientation of the camera based on device data
    ///
    bool updateCamera();

    ///
    /// \brief Module overrides
    ///
    bool init() override;
    void beginFrame() override;
    void endFrame() override;
    void exec() override;

private:

    vtkCamera* camera;                            //!< Pointer to rendering camera
    std::shared_ptr<DeviceInterface> inputDevice; //!< Pointer to input device
    std::chrono::milliseconds poolDelay;          //!< Pooling delay
    double bendingRadius;                         //!< bending radius of the tip of the scope
    double scalingFactor;                         //!< Scaling factor for physical to virtual translation

    double currentAngleX;
    double currentAngleY;
    double maxAngleX;
    double maxAngleY;
    double minAngleX;
    double minAngleY;
    double deltaAngleXY;
};

}

#endif // iSMTK_LAPAROSCOPIC_CAMERA_COUPLER_H
