/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkTrackingDeviceControl.h"

namespace imstk
{
class Camera;

///
/// \class CameraController
///
/// \brief Directly controls a camera given the device client pose, could be
/// unsmooth depending on device. Rigid body should be preferred for smoothness
///
class CameraController : public TrackingDeviceControl
{
public:
    CameraController(const std::string& name = "CameraController") : TrackingDeviceControl(name) { }
    ~CameraController() override = default;

    std::shared_ptr<Camera> getCamera() const { return m_camera; }
    void setCamera(std::shared_ptr<Camera> cam) { m_camera = cam; }

    ///
    /// \brief Set the offsets based on the current camera pose
    ///
    void setOffsetUsingCurrentCameraPose();

    ///
    /// \brief Updates the view of the provided camera
    ///
    void update(const double dt) override;

protected:
    std::shared_ptr<Camera> m_camera; ///< Camera controlled by the external device
};
} // namespace imstk