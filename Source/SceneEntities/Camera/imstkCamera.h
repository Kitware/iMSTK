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

#include <string>
#include <memory>

#include "imstkMath.h"

namespace imstk
{
///
/// \class Camera
///
/// \brief Generic camera class
///
class Camera
{
public:
    ///
    /// \brief Default contructor
    ///
    Camera() {}

    ///
    /// \brief Default destructor
    ///
    ~Camera() = default;

    ///
    /// \brief Gets the camera position
    /// \returns camera position
    ///
    const Vec3d& getPosition() const;

    ///
    /// \brief Sets the camera position
    ///
    void setPosition(const Vec3d& p);
    void setPosition(const double& x,
                     const double& y,
                     const double& z);

    ///
    /// \brief Returns the focal point
    ///        The focal point is the point that the camera points to
    /// \returns Focal point position
    ///
    const Vec3d& getFocalPoint() const;

    ///
    /// \brief Sets the focal point
    ///
    void setFocalPoint(const Vec3d& p);
    void setFocalPoint(const double& x,
                       const double& y,
                       const double& z);

    ///
    /// \brief Get the up vector
    /// \returns up vector of camera
    ///
    const Vec3d& getViewUp() const;

    ///
    /// \brief Set the up vector
    ///
    void setViewUp(const Vec3d& v);
    void setViewUp(const double& x,
                   const double& y,
                   const double& z);
    ///
    /// \brief Gets the field of view
    /// \returns vertical field of view in degrees
    ///
    const double& getFieldOfView() const;

    ///
    /// \brief Sets the field of view
    /// params fov vertical field of view in degrees
    ///
    void setFieldOfView(const double& fov);

protected:

    Vec3d  m_position    = Vec3d(0, 2, 5);          ///> camera position
    Vec3d  m_focalPoint  = WORLD_ORIGIN;            ///> camera focal point
    Vec3d  m_viewUp      = UP_VECTOR;               ///> camera up vector
    double m_fieldOfView = 60;                      ///> field of view in degrees
};
} // imstk
