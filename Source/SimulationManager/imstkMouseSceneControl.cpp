/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkMouseSceneControl.h"
#include "imstkCamera.h"
#include "imstkLogger.h"
#include "imstkMouseDeviceClient.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"

namespace imstk
{
void
MouseSceneControl::printControls()
{
    LOG(INFO) << "Mouse Scene Controls: Only usable in debug mode";
    LOG(INFO) << "----------------------------------------------------------------------";
    LOG(INFO) << " | Left click drag   - rotate view";
    LOG(INFO) << " | Middle click drag - pan view";
    LOG(INFO) << " | Scroll            - zoom in/out";
    LOG(INFO) << "----------------------------------------------------------------------";
}

void
MouseSceneControl::OnButtonPress(const int key)
{
    if (m_mode == Mode::None)
    {
        // Set the mode
        if (key == LEFT_BUTTON)
        {
            m_mode = Mode::Rotate;
        }
        else if (key == MIDDLE_BUTTON)
        {
            m_mode = Mode::Pan;
        }
    }
}

void
MouseSceneControl::OnButtonRelease(const int key)
{
    if (key == LEFT_BUTTON && m_mode == Mode::Rotate)
    {
        m_mode = Mode::None;
    }
    else if (key == MIDDLE_BUTTON && m_mode == Mode::Pan)
    {
        m_mode = Mode::None;
    }
}

void
MouseSceneControl::OnScroll(const double dx)
{
    if (!getEnabled())
    {
        return;
    }

    std::shared_ptr<Camera> cam     = m_sceneManager->getActiveScene()->getActiveCamera();
    const Vec3d&            focalPt = cam->getFocalPoint();
    const Vec3d&            camPos  = cam->getPosition();
    const Vec3d             diff    = camPos - focalPt;

    // Linear zoom function
    Vec3d newDiff;
    if (dx > 0.0)
    {
        newDiff = diff * 1.1 * m_zoomSpeed * m_zoomFactor;
    }
    else
    {
        newDiff = diff * 0.9 * m_zoomSpeed * m_zoomFactor;
    }
    const Vec3d newPos = focalPt + newDiff;
    cam->setPosition(newPos);
    cam->update();
}

void
MouseSceneControl::OnMouseMove(const Vec2d& pos)
{
    if (!getEnabled())
    {
        return;
    }

    std::shared_ptr<Camera> cam = m_sceneManager->getActiveScene()->getActiveCamera();

    // Push back the position
    m_prevPos = m_pos;
    m_pos     = pos;

    if (m_mode == Mode::Rotate)
    {
        // Map mouse deltas to theta, phi rotations on a sphere
        const Vec2d  dx     = (m_pos - m_prevPos) * (m_rotateSpeed * m_rotateFactor);
        const double dTheta = dx[1];  // Elevation
        const double dPhi   = -dx[0]; // Azimuth

        const Vec3d& focalPt     = cam->getFocalPoint();
        const Vec3d& camPos      = cam->getPosition();
        Vec3d        localCamPos = camPos - focalPt;

        // Get the rotation axes
        const Mat4d& view  = cam->getView();
        const Vec3d  up    = Vec3d(view(1, 0), view(1, 1), view(1, 2));
        const Vec3d  right = Vec3d(view(0, 0), view(0, 1), view(0, 2));
        // Rotate around each
        localCamPos = Rotd(dPhi, up).toRotationMatrix() * localCamPos;
        localCamPos = Rotd(dTheta, right).toRotationMatrix() * localCamPos;

        // Set the new cam pos and up, then compute lookat
        const Vec3d newPos = localCamPos + focalPt;
        cam->setPosition(newPos);
        cam->setViewUp(Vec3d(view(1, 0), view(1, 1), view(1, 2)));
        cam->update();
    }
    else if (m_mode == Mode::Pan)
    {
        // Move camera along up and right
        const Vec2d dx = m_pos - m_prevPos;

        const Vec3d& focalPt = cam->getFocalPoint();
        const Vec3d& camPos  = cam->getPosition();

        Mat4d&      view  = cam->getView();
        const Vec3d up    = Vec3d(view(1, 0), view(1, 1), view(1, 2));
        const Vec3d right = Vec3d(view(0, 0), view(0, 1), view(0, 2));

        // scale pan by zoom as well
        const double dist = (focalPt - camPos).norm();
        const Vec3d  dPos = (up * dx[1] + right * dx[0]) * -(m_panSpeed * m_panFactor * dist);
        cam->setFocalPoint(focalPt + dPos);
        cam->setPosition(camPos + dPos);
        cam->update();
    }
}

void
MouseSceneControl::setEnabled(bool enable)
{
    m_enabled = enable;
}

bool
MouseSceneControl::getEnabled() const
{
    return (m_sceneManager->getMode() == SceneManager::Mode::Debug) || m_enabled;
}
} // namespace imstk