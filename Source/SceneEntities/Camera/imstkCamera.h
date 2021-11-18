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

#include "imstkMath.h"

#include <iostream>

namespace imstk
{
///
/// \brief Produces a transform pointing forward towards target from position, oriented by up
///
static Mat4d
lookAt(const Vec3d& pos, const Vec3d& target, const Vec3d& up)
{
    Mat4d results = Mat4d::Identity();
    Mat3d R;
    R.col(2) = (pos - target).normalized();
    R.col(0) = up.cross(R.col(2)).normalized();
    R.col(1) = R.col(2).cross(R.col(0));
    results.topLeftCorner<3, 3>()  = R.transpose();
    results.topRightCorner<3, 1>() = -R.transpose() * pos;
    results(3, 3) = 1.0;

    return results;
}

///
/// \brief Produces a perpsective transformation matrix
///
//static Mat4d
//perspective(const double fovY, const double aspect, const double nearZ, const double farZ)
//{
//    Mat4d results;
//    results.setZero();
//    const double theta  = fovY * 0.5;
//    const double range  = (farZ - nearZ);
//    const double invTan = 1.0 / tan(theta);
//
//    results(0, 0) = invTan / aspect;
//    results(1, 1) = invTan;
//    results(2, 2) = -(nearZ + farZ) / range;
//    results(3, 2) = -1.0;
//    results(2, 3) = -2.0 * nearZ * farZ / range;
//    return results;
//}

///
/// \class Camera
///
/// \brief The base camera class defines a lookat camera
///
class Camera
{
public:
    Camera() = default;
    virtual ~Camera() = default;

    ///
    /// \brief Allows setting of the projection matrix without exposing a setter
    /// only renders may set it
    ///
    friend class VTKRenderer;

public:
    ///
    /// \brief Get camera view matrix
    /// \returns Camera view matrix reference
    ///
    Mat4d& getView() { return m_view; }

    ///
    /// \brief Get camera projection matrix, this matrix will
    /// be identity until first render is done
    /// \returns Camera projection matrix reference
    ///
    Mat4d& getProj() { return m_proj; }

    ///
    /// \brief Get the inverse view matrix
    ///
    const Mat4d& getInvView() { return m_invView; }

    ///
    /// \brief Set the camera view matrix
    ///
    void setView(const Mat4d& view)
    {
        m_viewModified = false;
        m_view    = view;
        m_invView = m_view.inverse();
    }

    ///
    /// \brief Gets the field of view
    /// \returns vertical field of view in degrees
    ///
    const double getFieldOfView() const { return m_fieldOfView; }

    ///
    /// \brief Sets the field of view
    /// \param vertical field of view in degrees
    ///
    void setFieldOfView(const double fov)
    {
        m_fieldOfView = fov;
        //m_projModified = true;
    }

    ///
    /// \brief Set clipping near
    /// note: You lose depth accuracy as the range between near and far increases
    /// could cause z fighting
    ///
    void setNearZ(const double nearZ) { m_nearZ = nearZ; }
    const double getNearZ() const { return m_nearZ; }

    ///
    /// \brief Set clipping near
    /// note: You lose depth accuracy as the range between near and far increases
    /// could cause z fighting
    ///
    void setFarZ(const double farZ) { m_farZ = farZ; }

    const double getFarZ() const { return m_farZ; }

    virtual void update()
    {
        if (m_viewModified)
        {
            m_view         = lookAt(m_position, m_focalPoint, m_viewUp);
            m_invView      = m_view.inverse();
            m_viewModified = false;
        }
        /*if (m_projModified)
        {
            m_proj = perspective();
        }*/
    }

    ///
    /// \brief Gets the camera position
    /// \returns camera position
    ///
    const Vec3d& getPosition() const { return m_position; }

    ///
    /// \brief Sets the camera position
    ///
    void setPosition(const Vec3d& pos)
    {
        m_position     = pos;
        m_viewModified = true;
    }

    void setPosition(const double x,
                     const double y,
                     const double z)
    {
        setPosition(Vec3d(x, y, z));
    }

    ///
    /// \brief Returns the focal point
    ///        The focal point is the point that the camera points to
    /// \returns Focal point position
    ///
    const Vec3d& getFocalPoint() const { return m_focalPoint; }

    ///
    /// \brief Sets the point to look at, point the camera towards
    ///
    void setFocalPoint(const Vec3d& focalPt)
    {
        m_focalPoint   = focalPt;
        m_viewModified = true;
    }

    void setFocalPoint(const double x,
                       const double y,
                       const double z)
    {
        setFocalPoint(Vec3d(x, y, z));
    }

    ///
    /// \brief Get the up direction of the view
    ///
    const Vec3d& getViewUp() const { return m_viewUp; }

    ///
    /// \brief Get the forward/look direction of the view
    ///
    const Vec3d getForward() const { return m_view.col(2).head<3>(); }

    ///
    /// \brief Compute ray emanating from the camera position that travels
    /// through the point in normalized device coordinates (-1,1 on x and y view plane)
    ///
    const Vec3d getEyeRayDir(const Vec2d& ndcPos) const
    {
        const Vec4d worldPos = (m_proj * m_view).inverse() * Vec4d(ndcPos[0], ndcPos[1], 0.0, 1.0);
        return (worldPos.head<3>() / worldPos[3] - m_position).normalized();
    }

    ///
    /// \brief Set the up vector
    ///
    void setViewUp(const Vec3d& up)
    {
        m_viewUp       = up.normalized();
        m_viewModified = true;
    }

    void setViewUp(const double x,
                   const double y,
                   const double z)
    {
        setViewUp(Vec3d(x, y, z));
    }

    ///
    /// \brief Utility function to quickly print cam stats
    ///
    void print();

protected:
    // Base camera values
    Mat4d m_view    = Mat4d::Identity(); ///> Actual view matrix used
    Mat4d m_invView = Mat4d::Identity(); ///> Inverse is often needed so we maintain it
    Mat4d m_proj;                        ///> Only modifiable through projection parameters (fov,near,far)
    bool  m_viewModified = true;

    // Base projection parameters
    double m_fieldOfView = 40.0; ///> field of view in degrees
    double m_nearZ       = 0.01; ///> near plane of the camera
    double m_farZ = 1000.0;      ///> far plane of the camera

protected:
    // Lookat camera parameters
    Vec3d m_position   = Vec3d(0.0, 2.0, 5.0); ///> camera position
    Vec3d m_focalPoint = Vec3d::Zero();        ///> camera focal point
    Vec3d m_viewUp     = Vec3d::UnitY();       ///> camera up vector
};
}
