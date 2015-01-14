#define GLM_FORCE_RADIANS
#include "smRendering/smCamera.h"

///
smCamera::smCamera()
{
    ar = 4.0f / 3.0f;
#ifdef GLM_FORCE_RADIANS
    angle = 0.785398f; //45 degrees
#else
    angle = 45.0f; //45 degrees
#endif
    nearClip = 0.1f;
    farClip = 100.0f;

    pos = glm::vec3(0, 0, 5);
    fp = glm::vec3(0, 0, 0);
    up = glm::vec3(0, 1, 0);
}

void smCamera::setCameraPos(float x, float y, float z)
{
    pos = glm::vec3(x, y, z);
}

void smCamera::setCameraPos(const glm::vec3& v)
{
    pos = v;
}

void smCamera::setCameraFocus(float x, float y, float z)
{
    fp = glm::vec3(x, y, z);
}

void smCamera::setCameraFocus(const glm::vec3& v)
{
    fp = v;
}

void smCamera::setCameraUpVec(float x, float y, float z)
{
    up = glm::vec3(x, y, z);
}

void smCamera::setCameraUpVec(const glm::vec3& v)
{
    up = v;
}

void smCamera::setAspectRatio(const float ar)
{
    this->ar = ar;
}

void smCamera::setViewAngle(const float a)
{
    angle = a;
}

void smCamera::setNearClipDist(const float d)
{
    nearClip = d;
}

void smCamera::setFarClipDist(const float d)
{
    farClip = d;
}

glm::mat4 smCamera::getViewMat()
{
    return view;
}

float* smCamera::getViewMatRef()
{
    return glm::value_ptr(view);
}

glm::mat4 smCamera::getProjMat()
{
    return proj;
}

float* smCamera::getProjMatRef()
{
    return glm::value_ptr(proj);
}

void smCamera::genViewMat()
{
    view = glm::lookAt(pos, fp, up);
}

void smCamera::genProjMat()
{
    proj = glm::perspective(angle, ar, nearClip, farClip);
}
