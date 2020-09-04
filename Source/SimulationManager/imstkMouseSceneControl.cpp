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

#include "imstkMouseSceneControl.h"
#include "imstkCamera.h"
#include "imstkLogger.h"
#include "imstkMouseDeviceClient.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"

namespace imstk
{
MouseSceneControl::MouseSceneControl(std::shared_ptr<MouseDeviceClient> device) : MouseControl(device)
{
}

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
    // If no mode currently selected
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
    // This control is disabled in simulation mode
    if (m_sceneManager->getMode() == SceneManager::Mode::Simulation)
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
    // Controls disabled in simulation mode
    if (m_sceneManager->getMode() == SceneManager::Mode::Simulation)
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
        const Vec3d dPos = (up * dx[1] + right * dx[0]) * -(m_panSpeed * m_panFactor * dist);
        cam->setFocalPoint(focalPt + dPos);
        cam->setPosition(camPos + dPos);
        cam->update();
    }
}

void
MouseSceneControl::update()
{
    // Directly set it
    //m_camera->getView() = m_targetViewTransform;

    // Slerp the camera rotation
    /*const Quatd currOrientation = Quatd(m_camera->getView().block<3, 3>(0, 0)).normalized();
    const Quatd targetOrientation = Quatd(m_targetViewTransform.block<3, 3>(0, 0)).normalized();
    const Quatd newOrientation = currOrientation.slerp(0.1, targetOrientation).normalized();
    m_camera->getView().block<3, 3>(0, 0) = newOrientation.toRotationMatrix();*/
}
}