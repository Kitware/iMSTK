/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCameraController.h"
#include "imstkCamera.h"
#include "imstkLogger.h"

namespace imstk
{
void
CameraController::update(const double& dt)
{
    if (!updateTrackingData(dt))
    {
        LOG(WARNING) << "warning: could not update tracking info.";
        return;
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
} // namespace imstk