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
FLSCameraController::setCameraAngulation(const double angle)
{
  m_cameraAngulation = angle;
  m_cameraAngulationRotOffset = Quatd(Eigen::AngleAxisd(angle*PI / 180., Vec3d(0., 1., 0.)));
}

double
FLSCameraController::getCameraAngulation() const
{
  return m_cameraAngulation;
  }

void 
FLSCameraController::setArduinoDevice(std::shared_ptr<VRPNArduinoDeviceClient> aClient)
{
  arduinoActive = true;
  arduinoClient = aClient;
  
}

void
FLSCameraController::runModule()
{

    //Get head angle from Arduino, performing calibration if this is the first valid report
    if (arduinoActive & calibrated)
    {
      this->setCameraHeadAngleOffset(arduinoClient->getRoll() - m_rollOffset);
    }
    else if (arduinoActive)
    {
      if (arduinoClient->getRoll() != 0){
        std::cout << "FLS Camera Controller:  Calibration complete; Safe to move camera" << std::endl;
        m_rollOffset = arduinoClient->getRoll();
        calibrated = true;
      }
    }
    auto roff = Quatd(Eigen::AngleAxisd(m_cameraHeadAngleOffset*PI / 180., Vec3d(0., 0., 1.)));
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


    //Adjust the upward angulation position to the center of the ROM
    auto angulationDirectionOffset = Quatd(Eigen::AngleAxisd(-90*PI / 180., Vec3d(0., 0., 1.)));
    r *= angulationDirectionOffset;


    // Apply Offsets over the device pose
    p += m_cameraTranslationOffset;      // Offset the device position

    //Apply offset from angulation
    r *= m_cameraAngulationRotOffset;

    // Set camera pose
    m_camera.setPosition(p);                                         //position of camera
    m_camera.setFocalPoint(r*FORWARD_VECTOR + p);                    //direction camera is looking
    m_camera.setViewUp(m_cameraRotationalOffset*UP_VECTOR);          //Orientation of camera

    m_trackingDataUptoDate = false;
}
} // imstk
