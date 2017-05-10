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
FLSCameraController::setCameraAngulationOffset(const Quatd& r)
{
    m_cameraAngulationOffset = r;
}

const imstk::Quatd&
FLSCameraController::getCameraAngulationOffset() const
{
    return m_cameraAngulationOffset;
}

void
FLSCameraController::runModule()
{
    auto roff = Quatd(Eigen::AngleAxisd(m_cameraHeadAngleOffset*PI / 180., Vec3d(0., 0., 1.)));
    roff *= m_cameraAngulationOffset;
    this->setRotationOffset(roff);

    CameraController::runModule();
}
} // imstk
