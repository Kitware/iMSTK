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

#include "imstkFLSCameraController.h"

#include <utility>

#include <g3log/g3log.hpp>

namespace imstk
{
void
FLSCameraController::setCameraHeadAngleOffset(const double angle)
{
    m_cameraHeadAngleOffset = angle;
}

const double
FLSCameraController::getCameraHeadAngleOffset() const
{
    return m_cameraHeadAngleOffset;
}

void
FLSCameraController::setCameraAngulationOffset(const double angle)
{
    m_cameraAngulationOffset = angle;
    m_cameraAngulationRotOffset = Quatd(Eigen::AngleAxisd(angle*PI / 180., Vec3d(0., 1., 0.)));
}

double
FLSCameraController::getCameraAngulationOffset() const
{
    return m_cameraAngulationOffset;
}

void 
FLSCameraController::setArduinoDevice(std::shared_ptr<VRPNArduinoDeviceClient> aClient)
{
  arduinoActive = true;
  arduinoClient = aClient;
  
  m_rollOffset = arduinoClient->getRoll();
}

void
FLSCameraController::setCameraAngulationTranslationOffset(const double t)
{
    m_angulationTranslationOffset = t;
}

double
FLSCameraController::getCameraAngulationTranslationOffset() const
{
    return m_angulationTranslationOffset;
}

void
FLSCameraController::runModule()
{
    if (arduinoActive)
    {
      this->setCameraHeadAngleOffset(arduinoClient->getRoll() - m_rollOffset);
    }
    auto roff = Quatd(Eigen::AngleAxisd(m_cameraHeadAngleOffset*PI / 180., Vec3d(0., 0., 1.)));
    roff *= m_cameraAngulationRotOffset;
    this->setCameraRotationOffset(roff);

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

    m_cameraTranslationOffset = r*Vec3d(m_angulationTranslationOffset*cos(m_cameraAngulationOffset*PI / 180.),
                                        0.,
                                        m_angulationTranslationOffset*sin(m_cameraAngulationOffset*PI / 180.));

    // Apply Offsets over the device pose
    p += m_cameraTranslationOffset;      // Offset the device position
    r *= m_cameraRotationalOffset;       // Apply camera head rotation offset

    // Set camera pose
    m_camera.setPosition(p);
    m_camera.setFocalPoint(r*FORWARD_VECTOR + p);
    m_camera.setViewUp(r*UP_VECTOR);

    m_trackingDataUptoDate = false;
}
} // imstk
