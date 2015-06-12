#ifndef SMCAMERA_H
#define SMCAMERA_H

#include "smUtilities/smMatrix.h"
#include "smUtilities/smVector.h"
#include "smUtilities/smQuaternion.h"

/// STL icludes
#include <memory>

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
    void setPos(float x, float y, float z);
    void setPos(const smVec3f& v);
    smVec3f getPos();
    void setFocus(float x, float y, float z);
    void setFocus(const smVec3f& v);
    smVec3f getFocus();
    smVec3f getUpVec();
    smVec3f getDirection();

    //Projection settings
    void setAspectRatio(const float ar);
    float getAspectRatio();
    void setViewAngle(const float a);
    float getViewAngle();
    void setViewAngleDeg(const float a);
    float getViewAngleDeg();
    void setNearClipDist(const float d);
    float getNearClipDist();
    void setFarClipDist(const float d);
    float getFarClipDist();

    //Set Matrix data
    void setViewMat(const smMatrix44f &m);
    void setProjMat(const smMatrix44f &m);
    smMatrix44f getViewMat();
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
    void rotateLocal(float angle, smVec3f axis);

    /// \brief Rotates about an arbitrary axis in focus point coordinates
    ///
    /// \param angle The angle in radians to rotate
    /// \param axis The axis to rotate about
    void rotateFocus(float angle, smVec3f axis);

    /// \brief Rotates about an arbitrary axis in global coordinates
    ///
    /// \param angle The angle in radians to rotate
    /// \param axis The axis to rotate about
    void rotateGlobal(float angle, smVec3f axis);

    /// \brief Rotates about the camera's local X axis
    ///
    /// \detail The camera local X axis is interpreted as the cross product of
    /// the directional vector from the camera position to the camera focal
    /// point and the camera UP vector
    ///
    /// \param angle The angle in radians to rotate
    void rotateLocalX(float angle);

    /// \brief Rotates about the camera local Y axis
    ///
    /// \detail The camera local Y axis is interpreted as the camera UP vector
    ///
    /// \param angle The angle in radians to rotate
    void rotateLocalY(float angle);

    /// \brief Rotates about the camera local Z axis
    ///
    /// \detail The camera local Z axis is interpreted as the directional
    /// vector from the camera focal point to the camera position
    ///
    /// \param angle The angle in radians to rotate
    void rotateLocalZ(float angle);

    /// \brief Rotates about the camera focal point X axis
    ///
    /// \detail The camera focal point X axis is interpreted as the cross
    /// product of the directional vector from the camera position to the camera
    /// focal point and the camera UP vector
    ///
    /// \param angle The angle in radians to rotate
    void rotateFocusX(float angle);

    /// \brief Rotates about the camera focal point Y axis
    ///
    /// \detail The camera focal point Y axis is interpreted as the camera UP vector
    /// (but above the focal point)
    ///
    /// \param angle The angle in radians to rotate
    void rotateFocusY(float angle);

    /// \brief Rotates about the camera focal point Z axis
    ///
    /// \detail The camera focal point Z axis is interpreted as the directional
    /// vector from the camera focal point to the camera position
    ///
    /// \param angle The angle in radians to rotate
    void rotateFocusZ(float angle);

    /// \brief Rotates about the global X axis
    ///
    /// \param angle The angle in radians to rotate
    void rotateGlobalX(float angle);

    /// \brief Rotates about the global Y axis
    ///
    /// \param angle The angle in radians to rotate
    void rotateGlobalY(float angle);

    /// \brief Rotates about the global Z axis
    ///
    /// \param angle The angle in radians to rotate
    void rotateGlobalZ(float angle);

    smMatrix44f lookAt(const smVec3f& pos,
                      const smVec3f& fp,
                      const smVec3f& up);
    smMatrix44f perspective(float fovy, float ar, float zNear, float zFar);

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

    static std::shared_ptr<smCamera> getDefaultCamera()
    {
        std::shared_ptr<smCamera> defaultCamera = std::make_shared<smCamera>();
        defaultCamera->setAspectRatio(800.0 / 640.0); //Doesn't have to match screen resolution
        defaultCamera->setFarClipDist(1000);
        defaultCamera->setNearClipDist(0.001);
        defaultCamera->setViewAngle(0.785398f); //45 degrees
        defaultCamera->setPos(0, 0, 10);
        defaultCamera->viewDirty = true;
        defaultCamera->projDirty = true;
        return defaultCamera;
    }

private:
    //View matrix variables
    smVec3f pos; ///< position of the camera
    smVec3f fp; ///< focal point of the camera
    smVec3f up; ///< the up direction for the camera
    smQuaternionf orientation; ///< orientation of the camera

    //Projection matrix variables
    float ar; ///< aspect ratio
    float angle; ///< angle in radians
    float nearClip; ///< near clipping distance
    float farClip; ///< far clipping distance

    //functional matrices
    smMatrix44f view; ///< View matrix for OpenGL
    smMatrix44f proj; ///< Projection matrix for OpenGL

    bool viewDirty; ///< True if view variables have been updated
    bool projDirty; ///< True if projection variables have been updated
    bool orientDirty; ///< True if orientation needs to be recalculated

    void setUpVec(float x, float y, float z);
    void setUpVec(const smVec3f& v);

    void setOrientFromDir(const smVec3f d);
    void setOrientation(const smQuaternionf q);
    smQuaternionf getOrientation();
};

#endif
