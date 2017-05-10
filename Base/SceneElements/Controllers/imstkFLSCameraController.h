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

#ifndef imstkFLSCameraController_h
#define imstkFLSCameraController_h

#include "imstkCameraController.h"

namespace imstk
{
///
/// \class FLSCameraController
///
/// \brief Custom camera controller for the virtual FLS camera navigation
///
class FLSCameraController : public CameraController
{
public:
    ///
    /// \brief
    ///
    FLSCameraController(Camera& camera,
                        std::shared_ptr<DeviceClient> deviceClient) :
        CameraController(camera, deviceClient)
    {}

    ///
    /// \brief Destructor
    ///
    ~FLSCameraController() = default;

    ///
    /// \brief Get/Set the camera head angular offset
    ///
    void setCameraHeadAngleOffset(const double angle);
    const double getCameraHeadAngleOffset() const;

    ///
    /// \brief Get/Set the rotation offset due to angulation
    ///
    void setCameraAngulationOffset(const Quatd& r);
    const Quatd& getCameraAngulationOffset() const;

protected:

    ///
    /// \brief
    ///
    void runModule() override;

    double m_cameraHeadAngleOffset = 0;                   ///< camera head angle offset (in deg)
    Quatd m_cameraAngulationOffset = Quatd::Identity();   ///< Rotation offset for the camera via telescope angulation
};
} // imstk

#endif // ifndef imstkFLSCameraController_h