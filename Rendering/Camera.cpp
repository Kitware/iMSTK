// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
//
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

#include "Core/Quaternion.h"
#include "Core/Vector.h"
#include "Rendering/Camera.h"

namespace imstk {

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

//---------------------------------------------------------------------------
const Vec3f &Camera::getPos() const
{
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(this->posLock));
    return this->pos;
}

//---------------------------------------------------------------------------
void Camera::setPos(const float &x, const float &y, const float &z)
{
    this->setPos(Vec3f(x, y, z));
}

void Camera::setPos(const Vec3f &v)
{
    {//scoped for mutex release
        std::lock_guard<std::mutex> lock(this->posLock);
        this->pos = v;
    }
    this->viewDirty.store(true);
    this->orientDirty.store(true);
}

const Vec3f &Camera::getFocus() const
{
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(this->fpLock));
    return this->fp;
}

void Camera::setFocus(const float &x, const float &y, const float &z)
{
    this->setFocus(Vec3f(x, y, z));
}

void Camera::setFocus(const Vec3f &v)
{
    { //scoped for mutex release
        std::lock_guard<std::mutex> lock(this->fpLock);
        this->fp = v;
    }
    this->viewDirty.store(true);
    this->orientDirty.store(true);
}

const Vec3f &Camera::getUpVec()
{
    this->upVector = getOrientation() * Vec3f::UnitY();
    return this->upVector;
}

const Vec3f &Camera::getDirection()
{
    this->direction = -(getOrientation() * Vec3f::UnitZ());
    return this->direction;
}

float Camera::getAspectRatio() const
{
    return this->ar.load();
}

void Camera::setAspectRatio(const float &ar)
{
    this->ar.store(ar);
    this->projDirty.store(true);
}

float Camera::getViewAngle() const
{
    return this->angle.load();
}

void Camera::setViewAngle(const float &a)
{
    this->angle.store(a);
    this->projDirty.store(true);
}

float Camera::getViewAngleDeg() const
{
    // Return degrees
    return 57.2957795130823*getViewAngle();
}

void Camera::setViewAngleDeg(const float &a)
{
    // Use radians
    setViewAngle(0.0174532925199433*a);
}

float Camera::getNearClipDist() const
{
    return this->nearClip.load();
}

void Camera::setNearClipDist(const float &d)
{
    this->nearClip.store(d);
    this->projDirty.store(true);
}

float Camera::getFarClipDist() const
{
    return this->farClip.load();
}

void Camera::setFarClipDist(const float &d)
{
    this->farClip.store(d);
    this->projDirty.store(true);
}

void Camera::setOrientation(const Quaternionf &q)
{
    { //scoped for mutex release
    std::lock_guard<std::mutex> lock(orientationLock);
    this->orientation = q;
    }
    this->orientDirty.store(false);
}

void Camera::setOrientFromDir(const Vec3f &d)
{
    Matrix33f camAxes;
    Vec3f tempUp;
    { //scoped for mutex release
        std::lock_guard<std::mutex> lock(orientationLock);
        tempUp = this->orientation * Vec3f::UnitY();
    }

    camAxes.col(2) = (-d).normalized();
    camAxes.col(0) = tempUp.cross( camAxes.col(2) ).normalized();
    camAxes.col(1) = camAxes.col(2).cross( camAxes.col(0) ).normalized();
    this->setOrientation(Quaternionf(camAxes));
}

const Quaternionf &Camera::getOrientation()
{
    if (this->orientDirty.load())
    {
        this->setOrientFromDir((this->getFocus() - this->getPos()).normalized());
    }
    std::lock_guard<std::mutex> lock(this->orientationLock);
    return this->orientation;
}

const Matrix44f &Camera::getViewMat()
{
    if (this->viewDirty.load())
    {
        this->genViewMat();
    }
    std::lock_guard<std::mutex> lock(this->viewLock);
    return this->view;
}

void Camera::setViewMat(const Matrix44f &m)
{
    { //scoped for mutex release
        std::lock_guard<std::mutex> lock(this->viewLock);
        this->view = m;
    }
    this->viewDirty.store(false);
}

const Matrix44f &Camera::getProjMat()
{
    if (this->projDirty.load())
    {
        this->genProjMat();
    }
    std::lock_guard<std::mutex> lock(this->projLock);
    return this->proj;
}

void Camera::setProjMat(const Matrix44f &m)
{
    { //scoped for mutex release
        std::lock_guard<std::mutex> lock(this->projLock);
        this->proj = m;
    }
    this->projDirty.store(false);
}

Vec3f Camera::pan(const Vec3f &v)
{
    auto u = this->getOrientation() * v;
    this->setPos(this->getPos() + u);
    this->setFocus(this->getFocus() + u);
    return u;
}

void Camera::zoom(const float &d)
{
    float dist = (this->getPos() - this->getFocus()).norm();
    if (dist > d)
    {
        this->setPos(this->getPos() + this->getDirection() * d);
    }
}

void Camera::rotateLocal(const float &angle, const Vec3f &axis)
{
    float dist = (this->getPos() - this->getFocus()).norm();
    Quaternionf q;
    q = Eigen::AngleAxisf(angle, axis.normalized());

    setOrientation(this->getOrientation() * q);
    setFocus(this->getPos() + dist * this->getDirection());
}

void Camera::rotateFocus(const float &angle, const Vec3f &axis)
{
    float dist = (this->getFocus() - this->getPos()).norm();
    Quaternionf q;
    q = Eigen::AngleAxisf(angle, axis.normalized());

    setOrientation(this->getOrientation() * q);
    setPos(this->getFocus() + dist * this->getDirection());
}

void Camera::rotateLocalX(const float &angle)
{
    rotateLocal(angle, Vec3f::UnitX());
}

void Camera::rotateLocalY(const float &angle)
{
    rotateLocal(angle, Vec3f::UnitY());
}

void Camera::rotateLocalZ(const float &angle)
{
    rotateLocal(angle, Vec3f::UnitZ());
}

void Camera::rotateFocusX(const float &angle)
{
    rotateFocus(angle, Vec3f::UnitX());
}

void Camera::rotateFocusY(const float &angle)
{
    rotateFocus(angle, Vec3f::UnitY());
}

void Camera::rotateFocusZ(const float &angle)
{
    rotateFocus(angle, Vec3f::UnitZ());
}

Matrix44f Camera::lookAt(const Vec3f &pos,
                         const Vec3f &fp,
                         const Vec3f &up) const
{
    Vec3f f = (fp - pos).normalized();
    Vec3f u = up.normalized();
    Vec3f s = f.cross(u).normalized();
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
    this->setViewMat(Camera::lookAt(getPos(), getFocus(), getUpVec()));
}

Matrix44f Camera::perspective(const float &fovy, const float &ar,
                              const float &zNear, const float &zFar) const
{
    assert(ar > 0);
    assert(zFar > zNear);

    double tanHalfFovy = std::tan(fovy / 2.0);
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
    this->setProjMat(Camera::perspective(getViewAngle(), getAspectRatio(),
                                         getNearClipDist(), getFarClipDist()));
}

}
