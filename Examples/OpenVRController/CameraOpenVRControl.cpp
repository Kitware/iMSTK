/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "CameraOpenVRControl.h"
#include "imstkCamera.h"
#include "imstkLogger.h"
#include "imstkOpenVRDeviceClient.h"

using namespace imstk;

void
CameraOpenVRControl::printControls()
{
    LOG(INFO) << "Mouse Scene Controls: Only usable in debug mode";
    LOG(INFO) << "----------------------------------------------------------------------";
    LOG(INFO) << " | Left Trackpad   - rotate view";
    LOG(INFO) << " | Right Trakcpad  - translate view";
    LOG(INFO) << "----------------------------------------------------------------------";
}

void
CameraOpenVRControl::update(const double dt)
{
    // We may switch cameras on the controller
    if (m_camera == nullptr)
    {
        return;
    }

    if (m_rotateDevice != nullptr)
    {
        const Vec2d& pos  = m_rotateDevice->getTrackpadPosition();
        const Mat4d& view = m_camera->getView();
        m_camera->setView(view * mat4dRotation(Rotd(-pos[0] * m_rotateSpeedScale * dt, Vec3d(0.0, 1.0, 0.0))));
    }
    if (m_translateDevice != nullptr)
    {
        const Vec2d& pos  = m_translateDevice->getTrackpadPosition();
        const Mat4d& view = m_camera->getView();
        m_camera->setView(view * mat4dTranslate(Vec3d(pos[0], 0.0, -pos[1]) * m_translateSpeedScale * dt));
    }
}