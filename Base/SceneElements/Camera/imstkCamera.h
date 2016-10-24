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

#ifndef imstkCamera_h
#define imstkCamera_h

#include <string>
#include <memory>

#include "imstkMath.h"

namespace imstk
{

class CameraController;
class DeviceClient;

///
/// \class Camera
///
/// \brief Generic camera class
///
class Camera
{
public:
    ///
    /// \brief
    ///
    Camera() {}

    ///
    /// \brief
    ///
    ~Camera() = default;

    ///
    /// \brief
    ///
    const Vec3d& getPosition() const;

    ///
    /// \brief
    ///
    void setPosition(const Vec3d& p);
    void setPosition(const double& x,
                     const double& y,
                     const double& z);

    ///
    /// \brief
    ///
    const Vec3d& getFocalPoint() const;

    ///
    /// \brief
    ///
    void setFocalPoint(const Vec3d& p);
    void setFocalPoint(const double& x,
                       const double& y,
                       const double& z);

    ///
    /// \brief
    ///
    const Vec3d& getViewUp() const;

    ///
    /// \brief
    ///
    void setViewUp(const Vec3d& v);

    ///
    /// \brief
    ///
    void setViewUp(const double& x,
                   const double& y,
                   const double& z);
    ///
    /// \brief
    ///
    const double& getViewAngle() const;

    ///
    /// \brief
    ///
    void setViewAngle(const double& angle);

    ///
    /// \brief
    ///
    std::shared_ptr<CameraController> getController() const;

    ///
    /// \brief
    ///
    std::shared_ptr<CameraController> setupController(std::shared_ptr<DeviceClient> deviceClient);

protected:
    std::shared_ptr<CameraController> m_controller;   ///>

    Vec3d m_position = Vec3d(0,2,5);    ///>
    Vec3d m_focalPoint = WORLD_ORIGIN;  ///>
    Vec3d m_viewUp = UP_VECTOR;         ///>
    double m_viewAngle = 60;            ///>
};

} // imstk

#endif // ifndef imstkCamera_h
