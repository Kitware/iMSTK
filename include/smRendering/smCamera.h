// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#ifndef SMCAMERA_H
#define SMCAMERA_H

#include "smUtilities/smMatrix.h"
#include "smUtilities/smVector.h"
#include "smUtilities/smQuaternion.h"

/// STL icludes
#include <memory>
#include <mutex>

/// \brief A simple camera class to calculate view and projection matrices
///
/// \detail The camera will be oriented facing down the -Z axis with an +Y axis
/// up vector
class smCamera
{
public:
    //Construction/Destruction
    smCamera();

    //View settings
    /// \brief Set the position of the camera
    ///
    /// \param x The position on the X axis in 3D space
    /// \param y The position on the Y axis in 3D space
    /// \param z The position on the Z axis in 3D space
    void setPos(const float x, const float y, const float z);

    /// \brief Set the position of the camera
    ///
    /// \param v 3 Dimensional vector describing the position of the camera
    void setPos(const smVec3f& v);

    /// \brief Get the position of the camera
    ///
    /// \return 3 Dimensional vector containing the position of the camera
    smVec3f getPos();

    /// \brief Set the focal point of the camera
    ///
    /// \param x The position on the X axis in 3D space
    /// \param y The position on the Y axis in 3D space
    /// \param z The position on the Z axis in 3D space
    void setFocus(const float x, const float y, const float z);

    /// \brief Set the focal point of the camera
    ///
    /// \param v 3 Dimensional vector describing the focal point of the camera
    void setFocus(const smVec3f& v);

    /// \brief Get the focal point of the camera
    ///
    /// \return 3 Dimensional vector containing the focal point of the camera
    smVec3f getFocus();

    /// \brief Get the up direction of the camera
    ///
    /// \return 3 Dimensional vector containing the up direction of the camera
    smVec3f getUpVec();

    /// \brief Get the direction the camera is facing
    ///
    /// \return 3 Dimensional vector containing the direction of the camera
    smVec3f getDirection();

    //Projection settings
    /// \brief Set the aspect ratio of the camera
    ///
    /// \detail It may be wise to set this the same as your window/screen size.
    /// However, it does not have to be the same.
    ///
    /// \param ar The new aspect ratio of the camera
    void setAspectRatio(const float ar);

    /// \brief Get the aspect ratio of the camera
    ///
    /// \return The aspect ratio of the camera
    float getAspectRatio();

    /// \brief Set the vertical view angle of the camera
    ///
    /// \param a The angle to set the view angle of the camera in radians
    void setViewAngle(const float a);

    /// \brief Get the vertical view angle of the camera
    ///
    /// \return The view angle of the camera in radians
    float getViewAngle();

    /// \brief Set the vertical view angle of the camera
    ///
    /// \param a The angle to set the view angle of the camera in degrees
    void setViewAngleDeg(const float a);

    /// \brief Get the vertical view angle of the camera
    ///
    /// \return The view angle of the camera in degrees
    float getViewAngleDeg();

    /// \brief Set the clipping distance for objects near to the camera
    ///
    /// \param d The distance an object can be near to the camera before it is
    /// clipped from view
    void setNearClipDist(const float d);

    /// \brief Get the near clipping distance of the camera
    ///
    /// \return The near clipping distance
    float getNearClipDist();

    /// \brief Set the clipping distance for objects too far from the camera
    ///
    /// \param d The distance an object can be from the camera before it is
    /// clipped from view
    void setFarClipDist(const float d);

    /// \brief Get the far clipping distance of the camera
    ///
    /// \return The far clipping distance
    float getFarClipDist();

    /// \brief Returns the internal view matrix for the camera
    ///
    /// \return A 4 by 4 matrix containing the view matrix of the camera
    smMatrix44f getViewMat();

    /// \brief Returns the internal projection matrix for the camera
    ///
    /// \return A 4 by 4 matrix containing the projection matrix of the camera
    smMatrix44f getProjMat();

    /// \brief Pan the camera view
    ///
    /// \detail Will translate the camera and it's focal point by the provided
    /// amount
    ///
    /// \param v Vector describing how much to pan by in each direction
    void pan(const smVec3f v);

    /// \brief Zoom the camera
    ///
    /// \detail Moves the camera's position toward/away the focal point by the
    /// provided amount.  Will not pass through the focal point
    ///
    /// \param d Distance to move the camera's position by
    void zoom(const float d);

    /// \brief Rotates about an arbitrary axis in local coordinates
    ///
    /// \param angle The angle in radians to rotate
    /// \param axis The axis to rotate about
    void rotateLocal(const float angle, const smVec3f axis);

    /// \brief Rotates about an arbitrary axis in focus point coordinates
    ///
    /// \param angle The angle in radians to rotate
    /// \param axis The axis to rotate about
    void rotateFocus(const float angle, const smVec3f axis);

    /// \brief Rotates about the camera's local X axis
    ///
    /// \detail The camera local X axis is interpreted as the cross product of
    /// the directional vector from the camera position to the camera focal
    /// point and the camera UP vector
    ///
    /// \param angle The angle in radians to rotate
    void rotateLocalX(const float angle);

    /// \brief Rotates about the camera local Y axis
    ///
    /// \detail The camera local Y axis is interpreted as the camera UP vector
    ///
    /// \param angle The angle in radians to rotate
    void rotateLocalY(const float angle);

    /// \brief Rotates about the camera local Z axis
    ///
    /// \detail The camera local Z axis is interpreted as the directional
    /// vector from the camera focal point to the camera position
    ///
    /// \param angle The angle in radians to rotate
    void rotateLocalZ(const float angle);

    /// \brief Rotates about the camera focal point X axis
    ///
    /// \detail The camera focal point X axis is interpreted as the cross
    /// product of the directional vector from the camera position to the camera
    /// focal point and the camera UP vector
    ///
    /// \param angle The angle in radians to rotate
    void rotateFocusX(const float angle);

    /// \brief Rotates about the camera focal point Y axis
    ///
    /// \detail The camera focal point Y axis is interpreted as the camera UP vector
    /// (but above the focal point)
    ///
    /// \param angle The angle in radians to rotate
    void rotateFocusY(const float angle);

    /// \brief Rotates about the camera focal point Z axis
    ///
    /// \detail The camera focal point Z axis is interpreted as the directional
    /// vector from the camera focal point to the camera position
    ///
    /// \param angle The angle in radians to rotate
    void rotateFocusZ(const float angle);

    /// \brief Creates a view matrix for use with OpenGL
    ///
    /// \param pos Position of the camera
    /// \param fp Focal point of the camera
    /// \param up Upward facing direction of the camera
    ///
    /// \return A 4 by 4 matrix containing the view matrix
    smMatrix44f lookAt(const smVec3f pos,
                       const smVec3f fp,
                       const smVec3f up);

    /// \brief Creates a perspective matrix for use with OpenGL
    ///
    /// \param fovy Field of view in the Y direction
    /// \param ar Aspect ratio
    /// \param zNear The near clipping distance
    /// \param zFar The far clipping distance
    ///
    /// \return A 4 by 4 matrix containing the perspective matrix
    smMatrix44f perspective(const float fovy, const float ar,
                            const float zNear, const float zFar);

    //Create matrices
    /// \brief Generates the view matrix
    ///
    /// \detail If this function is not called by the user, the class will call
    /// it upon request for the matrix.  Optionally call this function to speed
    /// up access time to the matrix after modifications to affecting variables
    void genViewMat();

    /// \brief Generates the projection matrix
    ///
    /// \detail If this function is not called by the user, the class will call
    /// it upon request for the matrix.  Optionally call this function to speed
    /// up access time to the matrix after modifications to affecting variables
    void genProjMat();

    /// \brief Creates a default camera
    ///
    /// \detail Default settings are(not including constructor settings:
    /// aspect ratio = (800/640),
    /// far clipping distance = 1000,
    /// near clipping distance = 0.001, and
    /// position 10 units +Z away from origin(0, 0, 10)
    ///
    /// \return The shared_ptr containing the address of the allocated camera
    static std::shared_ptr<smCamera> getDefaultCamera()
    {
        std::shared_ptr<smCamera> defaultCamera = std::make_shared<smCamera>();
        defaultCamera->setAspectRatio(800.0 / 640.0); //Doesn't have to match screen resolution
        defaultCamera->setFarClipDist(1000);
        defaultCamera->setNearClipDist(0.001);
        defaultCamera->setPos(0, 0, 10);
        return defaultCamera;
    }

private:
    //View matrix variables
    smVec3f pos; ///< position of the camera
    std::mutex posLock; //< Controls access to the pos variable
    smVec3f fp; ///< focal point of the camera
    std::mutex fpLock; //< Controls access to the fp variable
    smQuaternionf orientation; ///< orientation of the camera
    std::mutex orientationLock; //< Controls access to the orientation variable
    smMatrix44f view; ///< View matrix for OpenGL
    std::mutex viewLock; //< Controls access to the view variable
    std::atomic_bool viewDirty; ///< True if view variables have been updated
    std::atomic_bool orientDirty; ///< True if orientation needs to be recalculated

    //Projection matrix variables
    std::atomic<float> ar; ///< aspect ratio
    std::atomic<float> angle; ///< angle in radians
    std::atomic<float> nearClip; ///< near clipping distance
    std::atomic<float> farClip; ///< far clipping distance
    smMatrix44f proj; ///< Projection matrix for OpenGL
    std::mutex projLock; //< Controls access to the proj variable
    std::atomic_bool projDirty; ///< True if projection variables have been updated

    /// \brief Replace the internal view matrix of the camera
    ///
    /// \detail Beware: This might break things, other internal data is not
    /// updated also
    ///
    /// \param m The new view matrix of the camera
    void setViewMat(const smMatrix44f &m);

    /// \brief Replace the internal projection matrix of the camera
    ///
    /// \detail Beware: This might break things, other internal data is not
    /// updated also
    ///
    /// \param m The new projection matrix of the camera
    void setProjMat(const smMatrix44f &m);

    /// \brief Sets the orientation  of the camera from a given direction
    ///
    /// \detail Generally used to orient the camera toward the focal point
    ///
    /// \param d Directional vector to orient the camera with
    void setOrientFromDir(const smVec3f d);

    /// \brief Replaces the internal quaternion with the one provided
    ///
    /// \param q New internal orientation quaternion
    void setOrientation(const smQuaternionf q);

    /// \brief Returns the orientation of the camera
    ///
    /// \return The orientation of the camera as a quaternion
    smQuaternionf getOrientation();
};

#endif
