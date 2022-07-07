/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#pragma once

#include "imstkDeviceControl.h"
#include "imstkMath.h"

namespace imstk
{
class Camera;
class OpenVRDeviceClient;
class SceneManager;
} // namespace imstk

using namespace imstk;

///
/// \class CameraOpenVRControl
///
/// \brief Controls pose of a camera given two trackpads from OpenVRDeviceClient's
///
///
class CameraOpenVRControl : public DeviceControl
{
public:
    CameraOpenVRControl(const std::string& name = "CameraOpenVRControl") : DeviceControl(name) { }
    ~CameraOpenVRControl() override = default;

public:
    ///
    /// \brief Get/Set how fast the camera translates
    ///
    void setTranslateSpeedScale(const double translateSpeedScale) { m_translateSpeedScale = translateSpeedScale; }
    double getTranslateSpeedScale() const { return m_translateSpeedScale; }

    ///
    /// \brief Get/Set how fast the camera rotates
    ///
    void setRotateSpeedScale(const double rotateSpeedScale) { m_rotateSpeedScale = rotateSpeedScale; }
    double getRotateSpeedScale() const { return m_rotateSpeedScale; }

    ///
    /// \brief Get/Set the device that can translate the camera
    ///
    void setTranslateDevice(std::shared_ptr<OpenVRDeviceClient> translateDevice) { m_translateDevice = translateDevice; }
    std::shared_ptr<OpenVRDeviceClient> getTranslateDevice() const { return m_translateDevice; }

    ///
    /// \brief Get/Set the device that can rotate the camera
    ///
    void setRotateDevice(std::shared_ptr<OpenVRDeviceClient> rotateDevice) { m_rotateDevice = rotateDevice; }
    std::shared_ptr<OpenVRDeviceClient> getRotateDevice() const { return m_rotateDevice; }

    ///
    /// \brief Get/Set the camera to be controlled
    ///
    void setCamera(std::shared_ptr<Camera> camera) { m_camera = camera; }
    std::shared_ptr<Camera> getCamera() const { return m_camera; }

public:
    void printControls() override;

    ///
    /// \brief Updates control based on current device state
    ///
    void update(const double dt) override;

protected:
    std::shared_ptr<OpenVRDeviceClient> m_translateDevice = nullptr;
    std::shared_ptr<OpenVRDeviceClient> m_rotateDevice    = nullptr;

    std::shared_ptr<Camera> m_camera = nullptr;

    // User changeable values
    double m_rotateSpeedScale    = 1.0;
    double m_translateSpeedScale = 1.0;
};