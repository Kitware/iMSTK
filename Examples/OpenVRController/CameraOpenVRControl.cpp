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