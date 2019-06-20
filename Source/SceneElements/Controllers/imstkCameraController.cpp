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

#include <utility>

#include <g3log/g3log.hpp>

namespace imstk
{
void
CameraController::initModule()
{
}

void
CameraController::runModule()
{
    if (!m_trackingDataUptoDate)
    {
        if (!updateTrackingData())
        {
            LOG(WARNING) << "CameraController::runModule warning: could not update tracking info.";
            return;
        }
    }

    Vec3d p = getPosition();
    Quatd r = getRotation();

    // Apply Offsets over the device pose
    p = p + m_cameraTranslationOffset;      // Offset the device position
    r *= m_cameraRotationalOffset;          // Apply camera head rotation offset

    // Set camera info
    m_camera.setPosition(p);
    m_camera.setFocalPoint((r * FORWARD_VECTOR) + p);
    m_camera.setViewUp(r * UP_VECTOR);

    m_trackingDataUptoDate = false;
}

void
CameraController::setOffsetUsingCurrentCameraPose()
{
    auto pos = m_camera.getPosition();
    auto viewUp = m_camera.getViewUp();
    auto focus = m_camera.getFocalPoint();

    m_translationOffset = pos;

    auto viewNormal = (pos - focus).normalized();
    auto viewSide = viewUp.cross(viewNormal).normalized();
    viewUp = viewNormal.cross(viewSide);
    Mat3d rot;
    rot.col(0) = viewSide;
    rot.col(1) = viewUp;
    rot.col(2) = viewNormal;
    m_rotationOffset = Quatd(rot);
}

void
CameraController::setCameraRotationOffset(const Quatd& r)
{
    m_cameraRotationalOffset = r;
}

void
CameraController::setCameraTranslationOffset(const Vec3d& t)
{
    m_cameraTranslationOffset = t;
}

const Vec3d&
CameraController::getCameraTranslationOffset() const
{
    return m_cameraTranslationOffset;
}

const Quatd&
CameraController::getCameraRotationOffset() const
{
    return m_cameraRotationalOffset;
}
} // imstk
