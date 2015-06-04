#include "smUtilities/smMath.h"
#include "smRendering/smCamera.h"

///
smCamera::smCamera()
{
    ar = 4.0 / 3.0;
    angle = SM_DEGREES2RADIANS(45.0);
    nearClip = 0.1;
    farClip = 100.0;

    pos = smVec3f(0, 0, 0);
    fp = smVec3f(0, 0, -1);
    up = smVec3f(0, 1, 0);
}

smVec3f smCamera::getPos()
{
    return pos;
}

void smCamera::setPos(float x, float y, float z)
{
    pos = smVec3f(x, y, z);
}

void smCamera::setPos(const smVec3f& v)
{
    pos = v;
}

smVec3f smCamera::getFocus()
{
    return fp;
}

void smCamera::setFocus(float x, float y, float z)
{
    fp = smVec3f(x, y, z);
}

void smCamera::setFocus(const smVec3f& v)
{
    fp = v;
}

smVec3f smCamera::getUpVec()
{
    return up;
}

void smCamera::setUpVec(float x, float y, float z)
{
    up = smVec3f(x, y, z);
}

void smCamera::setUpVec(const smVec3f& v)
{
    up = v;
}

float smCamera::getAspectRatio()
{
    return ar;
}

void smCamera::setAspectRatio(const float ar)
{
    this->ar = ar;
}

float smCamera::getViewAngle()
{
    return angle;
}

void smCamera::setViewAngle(const float a)
{
    angle = a;
}

float smCamera::getViewAngleDeg()
{
    return SM_RADIANS2DEGREES(angle);
}

void smCamera::setViewAngleDeg(const float a)
{
    angle = SM_DEGREES2RADIANS(a);
}

float smCamera::getNearClipDist()
{
    return nearClip;
}

void smCamera::setNearClipDist(const float d)
{
    nearClip = d;
}

float smCamera::getFarClipDist()
{
    return farClip;
}

void smCamera::setFarClipDist(const float d)
{
    farClip = d;
}

smMatrix44f smCamera::getViewMat()
{
    return view;
}

void smCamera::setViewMat(const smMatrix44f &m)
{
    this->view = m;
}

smMatrix44f smCamera::getProjMat()
{
    return proj;
}

void smCamera::setProjMat(const smMatrix44f &m)
{
    this->proj = m;
}

// Implementation adapted from Sylvain Pointeau's Blog:
// http://spointeau.blogspot.com/2013/12/hello-i-am-looking-at-opengl-3.html
smMatrix44f smCamera::lookAt(const smVec3f& pos,
                             const smVec3f& fp,
                             const smVec3f& up)
{
    smVec3f f = (fp - pos).normalized();
    smVec3f u = up.normalized();
    smVec3f s = f.cross(u).normalized();
    u = s.cross(f);

    smMatrix44f res;
    res <<  s.x(),s.y(),s.z(),-s.dot(pos),
            u.x(),u.y(),u.z(),-u.dot(pos),
            -f.x(),-f.y(),-f.z(),f.dot(pos),
            0,0,0,1;

    return res;
}

void smCamera::genViewMat()
{
    this->view = smCamera::lookAt(pos, fp, up);
}

// Implementation adapted from Sylvain Pointeau's Blog:
// http://spointeau.blogspot.com/2013/12/hello-i-am-looking-at-opengl-3.html
smMatrix44f smCamera::perspective(float fovy, float ar, float zNear, float zFar)
{
    assert(ar > 0);
    assert(zFar > zNear);

    double tanHalfFovy = tan(fovy / 2.0);
    smMatrix44f res = smMatrix44f::Zero();

    res(0,0) = 1.0 / (ar * tanHalfFovy);
    res(1,1) = 1.0 / (tanHalfFovy);
    res(2,2) = - (zFar + zNear) / (zFar - zNear);
    res(3,2) = - 1.0;
    res(2,3) = - (2.0 * zFar * zNear) / (zFar - zNear);
    return res;
}

void smCamera::genProjMat()
{
    proj = smCamera::perspective(angle, ar, nearClip, farClip);
}
