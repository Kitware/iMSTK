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

#include "imstkCameraController.h"
#include "imstkCamera.h"
#include "imstkLogger.h"

namespace imstk
{
CameraController::CameraController(std::shared_ptr<Camera>       camera,
                                   std::shared_ptr<DeviceClient> deviceClient) :
    TrackingDeviceControl(deviceClient),
    m_camera(camera)
{
}

void
CameraController::update(const double dt)
{
    if (!isTrackerUpToDate())
    {
        if (!updateTrackingData(dt))
        {
            LOG(WARNING) << "warning: could not update tracking info.";
            return;
        }
    }

    Vec3d p = getPosition();
    Quatd r = getOrientation();

    // Apply Offsets over the device pose
    p  = p + m_translationOffset;   // Offset the device position
    r *= m_rotationOffset;          // Apply camera head rotation offset

    // Set camera info
    m_camera->setPosition(p);
    m_camera->setFocalPoint((r * Vec3d(0.0, 0.0, -1.0)) + p);
    m_camera->setViewUp(r * Vec3d(0.0, 1.0, 0.0));
}

void
CameraController::setOffsetUsingCurrentCameraPose()
{
    const auto pos    = m_camera->getPosition();
    const auto focus  = m_camera->getFocalPoint();
    auto       viewUp = m_camera->getViewUp();

    m_translationOffset = pos;

    auto viewNormal = (pos - focus).normalized();
    auto viewSide   = viewUp.cross(viewNormal).normalized();
    viewUp = viewNormal.cross(viewSide);
    Mat3d rot;
    rot.col(0)       = viewSide;
    rot.col(1)       = viewUp;
    rot.col(2)       = viewNormal;
    m_rotationOffset = Quatd(rot);
}
}