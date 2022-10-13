/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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
/// \class VRCameraControl
///
/// \brief Defines a control scheme to move the camera with joysticks.
/// Left joystick look, right joystick move, a,b up/down
///
class VRCameraControl : public DeviceControl
{
public:
    VRCameraControl(const std::string& name = "VRCameraControl") : DeviceControl(name) { }
    ~VRCameraControl() override = default;

public:
    ///
    /// \brief Get/Set how fast the camera translates
    ///@{
    double getTranslateSpeedScale() const { return m_translateSpeedScale; }
    void setTranslateSpeedScale(const double translateSpeedScale)
    {
        m_translateSpeedScale = translateSpeedScale;
    }

    ///@}

    ///
    /// \brief Get/Set how fast the camera rotates
    ///@{
    double getRotateSpeedScale() const { return m_rotateSpeedScale; }
    void setRotateSpeedScale(const double rotateSpeedScale)
    {
        m_rotateSpeedScale = rotateSpeedScale;
    }

    ///@}

    ///
    /// \brief Get/Set the vertical speed
    ///@{
    double getVerticalSpeedScale() const { return m_translateVerticalSpeedScale; }
    void setVerticalSpeedScale(double verticalSpeed)
    {
        m_translateVerticalSpeedScale = verticalSpeed;
    }

    ///@}

    ///
    /// \brief Get/Set the device that can translate the camera
    ///@{
    std::shared_ptr<OpenVRDeviceClient> getTranslateDevice() const { return m_translateDevice; }
    void setTranslateDevice(std::shared_ptr<OpenVRDeviceClient> translateDevice)
    {
        m_translateDevice = translateDevice;
    }

    ///@}

    ///
    /// \brief Get/Set the device that can rotate the camera
    ///@{
    std::shared_ptr<OpenVRDeviceClient> getRotateDevice() const { return m_rotateDevice; }
    void setRotateDevice(std::shared_ptr<OpenVRDeviceClient> rotateDevice)
    {
        m_rotateDevice = rotateDevice;
    }

    ///@}

    const Mat4d& getDeltaTransform() { return m_deltaTransform; }

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
    void update(const double& dt) override;

protected:
    std::shared_ptr<OpenVRDeviceClient> m_translateDevice = nullptr;
    std::shared_ptr<OpenVRDeviceClient> m_rotateDevice    = nullptr;

    std::shared_ptr<Camera> m_camera = nullptr;

    // User changeable values
    double m_rotateSpeedScale    = 1.0;
    double m_translateSpeedScale = 1.0;
    double m_translateVerticalSpeedScale = 1.0;
    Mat4d  m_deltaTransform = Mat4d::Identity();
};