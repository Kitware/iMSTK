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

#pragma once

#include "GLFW/glfw3.h"

#include <iostream>
#include <unordered_map>
#include <functional>

#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"

#include "imstkMath.h"
#include "imstkTimer.h"
#include "imstkRenderer.h"
#include "imstkVulkanInteractorStyle.h"

namespace imstk
{
class VulkanViewer;
class SimulationManager;

///
/// \class VulkanInteractorStyleFreeCamera
///
/// \brief Default camera movement class
///
class VulkanInteractorStyleFreeCamera : public VulkanInteractorStyle
{
public:
    VulkanInteractorStyleFreeCamera();
    ~VulkanInteractorStyleFreeCamera(){};

    ///
    /// \brief Controls camera movement
    ///
    virtual void OnTimer();

protected:
    friend class VulkanViewer;

    double m_lastTime = 0; ///< Last frame time
    Vec3d m_simCameraPosition; ///< Saved simulation mode camera position
    Vec3d m_simCameraFocalPoint; ///< Saved simulation mode camera focal point
    float m_cameraAngle = 0; ///< Angle created by changes in vertical cursor position

    bool m_started = false; ///< Used to initialized variables
};
}
