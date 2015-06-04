#ifndef SMCAMERA_H
#define SMCAMERA_H

#include "smUtilities/smMatrix.h"
#include "smUtilities/smVector.h"

/// STL icludes
#include <memory>

/// \brief A simple camera class to calculate view and projection matrices
///
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
    void setUpVec(float x, float y, float z);
    void setUpVec(const smVec3f& v);
    smVec3f getUpVec();

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

    //Get matrix data
    smMatrix44f getViewMat();
    smMatrix44f getProjMat();

    //Set Matrix data
    void setViewMat(const smMatrix44f &m);
    void setProjMat(const smMatrix44f &m);

    //Create matrices
    void genViewMat();
    void genProjMat();

    static smMatrix44f lookAt(const smVec3f& pos,
                             const smVec3f& fp,
                             const smVec3f& up);
    static smMatrix44f perspective(float fovy, float ar, float zNear, float zFar);

    static std::shared_ptr<smCamera> getDefaultCamera()
    {
        std::shared_ptr<smCamera> defaultCamera = std::make_shared<smCamera>();
        defaultCamera->setAspectRatio(800.0 / 640.0); //Doesn't have to match screen resolution
        defaultCamera->setFarClipDist(1000);
        defaultCamera->setNearClipDist(0.001);
        defaultCamera->setViewAngle(0.785398f); //45 degrees
        defaultCamera->setPos(0, 0, 10);
        defaultCamera->setFocus(0, 0, 0);
        defaultCamera->setUpVec(0, 1, 0);
        defaultCamera->genProjMat();
        defaultCamera->genViewMat();
        return defaultCamera;
    }

private:
    //View matrix variables
    smVec3f pos; ///< position of the camera
    smVec3f fp; ///< focal point of the camera
    smVec3f up; ///< the up direction for the camera

    //Projection matrix variables
    float ar; ///< aspect ratio
    float angle; ///< angle in radians
    float nearClip; ///< near clipping distance
    float farClip; ///< far clipping distance

    //functional matrices
    smMatrix44f view; ///< View matrix for OpenGL
    smMatrix44f proj; ///< Projection matrix for OpenGL

};

#endif
