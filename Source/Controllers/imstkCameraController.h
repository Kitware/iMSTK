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
    CameraController(std::shared_ptr<Camera> camera, std::shared_ptr<DeviceClient> deviceClient);
    virtual ~CameraController() = default;

public:
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