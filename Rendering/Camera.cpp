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

#include "Core/Quaternion.h"
#include "Core/Vector.h"
#include "Rendering/Camera.h"

Camera::Camera()
    : pos(0, 0, 0),
      fp(0, 0, -1),
      ar(4.0 / 3.0),
      angle(M_PI_4),
      nearClip(0.1),
      farClip(100.0)
{
    viewDirty.store(true);
    orientDirty.store(false);
    projDirty.store(true);
}

core::Vec3f Camera::getPos()
{
    std::lock_guard<std::mutex> lock(posLock);
    return this->pos;
}

void Camera::setPos(const float x, const float y, const float z)
{
    this->setPos(core::Vec3f(x, y, z));
}

void Camera::setPos(const core::Vec3f& v)
{
    {//scoped for mutex release
    std::lock_guard<std::mutex> lock(posLock);
    this->pos = v;
    }
    this->viewDirty.store(true);
    this->orientDirty.store(true);
}

core::Vec3f Camera::getFocus()
{
    std::lock_guard<std::mutex> lock(fpLock);
    return this->fp;
}

void Camera::setFocus(const float x, const float y, const float z)
{
    this->setFocus(core::Vec3f(x, y, z));
}

void Camera::setFocus(const core::Vec3f& v)
{
    { //scoped for mutex release
    std::lock_guard<std::mutex> lock(fpLock);
    this->fp = v;
    }
    this->viewDirty.store(true);
    this->orientDirty.store(true);
}

core::Vec3f Camera::getUpVec()
{
    return getOrientation() * core::Vec3f::UnitY();
}

core::Vec3f Camera::getDirection()
{
    return -(getOrientation() * core::Vec3f::UnitZ());
}

float Camera::getAspectRatio()
{
    return this->ar.load();
}

void Camera::setAspectRatio(const float ar)
{
    this->ar.store(ar);
    this->projDirty.store(true);
}

float Camera::getViewAngle()
{
    return this->angle.load();
}

void Camera::setViewAngle(const float a)
{
    this->angle.store(a);
    this->projDirty.store(true);
}

float Camera::getViewAngleDeg()
{
    // Return degrees
    return 57.2957795130823*getViewAngle();
}

void Camera::setViewAngleDeg(const float a)
{
    // Use radians
    setViewAngle(0.0174532925199433*a);
}

float Camera::getNearClipDist()
{
    return this->nearClip.load();
}

void Camera::setNearClipDist(const float d)
{
    this->nearClip.store(d);
    this->projDirty.store(true);
}

float Camera::getFarClipDist()
{
    return this->farClip.load();
}

void Camera::setFarClipDist(const float d)
{
    this->farClip.store(d);
    this->projDirty.store(true);
}

void Camera::setOrientation(const Quaternionf q)
{
    { //scoped for mutex release
    std::lock_guard<std::mutex> lock(orientationLock);
    this->orientation = q;
    }
    this->orientDirty.store(false);
}

void Camera::setOrientFromDir(const core::Vec3f d)
{
    Matrix33f camAxes;
    core::Vec3f tempUp;
    { //scoped for mutex release
    std::lock_guard<std::mutex> lock(orientationLock);
    tempUp = this->orientation * core::Vec3f::UnitY();
    }

    camAxes.col(2) = (-d).normalized();
    camAxes.col(0) = tempUp.cross( camAxes.col(2) ).normalized();
    camAxes.col(1) = camAxes.col(2).cross( camAxes.col(0) ).normalized();
    setOrientation(Quaternionf(camAxes));
}

Quaternionf Camera::getOrientation()
{
    if (true == this->orientDirty.load())
    {
        setOrientFromDir((getFocus() - getPos()).normalized());
    }
    std::lock_guard<std::mutex> lock(orientationLock);
    return this->orientation;
}

Matrix44f Camera::getViewMat()
{
    if (true == this->viewDirty.load())
    {
        genViewMat();
    }
    std::lock_guard<std::mutex> lock(viewLock);
    return this->view;
}

void Camera::setViewMat(const Matrix44f &m)
{
    { //scoped for mutex release
    std::lock_guard<std::mutex> lock(viewLock);
    this->view = m;
    }
    this->viewDirty.store(false);
}

Matrix44f Camera::getProjMat()
{
    if (true == this->projDirty.load())
    {
        genProjMat();
    }
    std::lock_guard<std::mutex> lock(projLock);
    return this->proj;
}

void Camera::setProjMat(const Matrix44f &m)
{
    { //scoped for mutex release
    std::lock_guard<std::mutex> lock(projLock);
    this->proj = m;
    }
    this->projDirty.store(false);
}

void Camera::pan(core::Vec3f v)
{
    v = getOrientation() * v;
    setPos(getPos() + v);
    setFocus(getFocus() + v);
}

void Camera::zoom(const float d)
{
    float dist = (getPos() - getFocus()).norm();
    if (dist > d)
    {
        setPos(getPos() + getDirection() * d);
    }
}

void Camera::rotateLocal(const float angle, const core::Vec3f axis)
{
    float dist = (getPos() - getFocus()).norm();
    Quaternionf q;
    q = Eigen::AngleAxisf(angle, axis.normalized());

    setOrientation(getOrientation() * q);
    setFocus(getPos() + dist * getDirection());
}

void Camera::rotateFocus(const float angle, const core::Vec3f axis)
{
    float dist = (getFocus() - getPos()).norm();
    Quaternionf q;
    q = Eigen::AngleAxisf(angle, axis.normalized());

    setOrientation(getOrientation() * q);
    setPos(getFocus() + dist * getDirection());
}

void Camera::rotateLocalX(const float angle)
{
    rotateLocal(angle, core::Vec3f::UnitX());
}

void Camera::rotateLocalY(const float angle)
{
    rotateLocal(angle, core::Vec3f::UnitY());
}

void Camera::rotateLocalZ(const float angle)
{
    rotateLocal(angle, core::Vec3f::UnitZ());
}

void Camera::rotateFocusX(const float angle)
{
    rotateFocus(angle, core::Vec3f::UnitX());
}

void Camera::rotateFocusY(const float angle)
{
    rotateFocus(angle, core::Vec3f::UnitY());
}

void Camera::rotateFocusZ(const float angle)
{
    rotateFocus(angle, core::Vec3f::UnitZ());
}


// Implementation adapted from Sylvain Pointeau's Blog:
// http://spointeau.blogspot.com/2013/12/hello-i-am-looking-at-opengl-3.html
Matrix44f Camera::lookAt(const core::Vec3f pos,
                             const core::Vec3f fp,
                             const core::Vec3f up)
{
    core::Vec3f f = (fp - pos).normalized();
    core::Vec3f u = up.normalized();
    core::Vec3f s = f.cross(u).normalized();
    u = s.cross(f);

    Matrix44f res;
    res <<  s.x(),s.y(),s.z(),-s.dot(pos),
            u.x(),u.y(),u.z(),-u.dot(pos),
            -f.x(),-f.y(),-f.z(),f.dot(pos),
            0,0,0,1;

    return res;
}

void Camera::genViewMat()
{
    setViewMat(Camera::lookAt(getPos(), getFocus(), getUpVec()));
}

// Implementation adapted from Sylvain Pointeau's Blog:
// http://spointeau.blogspot.com/2013/12/hello-i-am-looking-at-opengl-3.html
Matrix44f Camera::perspective(const float fovy, const float ar,
                                  const float zNear, const float zFar)
{
    assert(ar > 0);
    assert(zFar > zNear);

    double tanHalfFovy = tan(fovy / 2.0);
    Matrix44f res = Matrix44f::Zero();

    res(0,0) = 1.0 / (ar * tanHalfFovy);
    res(1,1) = 1.0 / (tanHalfFovy);
    res(2,2) = - (zFar + zNear) / (zFar - zNear);
    res(3,2) = - 1.0;
    res(2,3) = - (2.0 * zFar * zNear) / (zFar - zNear);
    return res;
}

void Camera::genProjMat()
{
    setProjMat(Camera::perspective(getViewAngle(), getAspectRatio(),
                                     getNearClipDist(), getFarClipDist()));
}
