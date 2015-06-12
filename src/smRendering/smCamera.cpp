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

#include "smUtilities/smMath.h"
#include "smUtilities/smQuaternion.h"
#include "smUtilities/smVector.h"
#include "smRendering/smCamera.h"

smCamera::smCamera()
{
    ar = 4.0 / 3.0;
    angle = SM_DEGREES2RADIANS(45.0);
    nearClip = 0.1;
    farClip = 100.0;

    pos = smVec3f(0, 0, 0);
    fp = smVec3f(0, 0, -1);

    orientation = smQuaternionf();

    viewDirty = true;
    projDirty = true;
    orientDirty = false;
}

smVec3f smCamera::getPos()
{
    return pos;
}

void smCamera::setPos(float x, float y, float z)
{
    this->setPos(smVec3f(x, y, z));
    orientDirty = true;
}

void smCamera::setPos(const smVec3f& v)
{
    pos = v;
    viewDirty = true;
}

smVec3f smCamera::getFocus()
{
    return fp;
}

void smCamera::setFocus(float x, float y, float z)
{
    this->setFocus(smVec3f(x, y, z));
    orientDirty = true;
}

void smCamera::setFocus(const smVec3f& v)
{
    fp = v;
    viewDirty = true;
}

smVec3f smCamera::getUpVec()
{
    return getOrientation() * smVec3f::UnitY();
}

smVec3f smCamera::getDirection()
{
    return -(getOrientation() * smVec3f::UnitZ());
}

float smCamera::getAspectRatio()
{
    return ar;
}

void smCamera::setAspectRatio(const float ar)
{
    this->ar = ar;
    projDirty = true;
}

float smCamera::getViewAngle()
{
    return angle;
}

void smCamera::setViewAngle(const float a)
{
    angle = a;
    projDirty = true;
}

float smCamera::getViewAngleDeg()
{
    return SM_RADIANS2DEGREES(angle);
}

void smCamera::setViewAngleDeg(const float a)
{
    angle = SM_DEGREES2RADIANS(a);
    projDirty = true;
}

float smCamera::getNearClipDist()
{
    return nearClip;
}

void smCamera::setNearClipDist(const float d)
{
    nearClip = d;
    projDirty = true;
}

float smCamera::getFarClipDist()
{
    return farClip;
}

void smCamera::setFarClipDist(const float d)
{
    farClip = d;
    projDirty = true;
}

void smCamera::setOrientation(const smQuaternionf q)
{
    this->orientation = q;
}

void smCamera::setOrientFromDir(const smVec3f d)
{
    smMatrix33f camAxes;
    smVec3f tempUp = this->orientation * smVec3f::UnitY();

    camAxes.col(2) = (-d).normalized();
    camAxes.col(0) = tempUp.cross( camAxes.col(2) ).normalized();
    camAxes.col(1) = camAxes.col(2).cross( camAxes.col(0) ).normalized();
    setOrientation(smQuaternionf(camAxes));
}

smQuaternionf smCamera::getOrientation()
{
    if (true == orientDirty)
    {
        setOrientFromDir(getFocus() - getPos());
        orientDirty = false;
    }
    return this->orientation;
}

smMatrix44f smCamera::getViewMat()
{
    if (true == viewDirty)
    {
        return view;
    }
    genViewMat();
    return view;
}

void smCamera::setViewMat(const smMatrix44f &m)
{
    this->view = m;
}

smMatrix44f smCamera::getProjMat()
{
    if (true == projDirty)
    {
        return proj;
    }
    genProjMat();
    return proj;
}

void smCamera::setProjMat(const smMatrix44f &m)
{
    this->proj = m;
}

void smCamera::pan(smVec3f v)
{
    v = getOrientation() * v;
    setPos(getPos() + v);
    setFocus(getFocus() + v);
}

void smCamera::zoom(const float d)
{
    float dist = (getPos() - getFocus()).norm();
    if (dist > d)
    {
        setPos(getPos() + getDirection() * d);
    }
}

void smCamera::rotateLocal(float angle, smVec3f axis)
{
    float dist = (getPos() - getFocus()).norm();
    smQuaternionf q;
    q = Eigen::AngleAxisf(angle, axis.normalized());

    setOrientation(getOrientation() * q);
    setFocus(getPos() + dist * getOrientation() * getDirection());
}

void smCamera::rotateFocus(float angle, smVec3f axis)
{
    float dist = (getFocus() - getPos()).norm();
    smQuaternionf q;
    q = Eigen::AngleAxisf(angle, axis.normalized());

    setOrientation(getOrientation() * q);
    setPos(getFocus() + dist * getOrientation() * getDirection());
}

void smCamera::rotateGlobal(float angle, smVec3f axis)
{
    float dist = (getFocus() - getPos()).norm();
    smQuaternionf q;
    q = Eigen::AngleAxisf(angle, axis.normalized());

    setPos(getPos() + dist * q * getDirection());
}

void smCamera::rotateLocalX(float angle)
{
    rotateLocal(angle, smVec3f::UnitX());
}

void smCamera::rotateLocalY(float angle)
{
    rotateLocal(angle, smVec3f::UnitY());
}

void smCamera::rotateLocalZ(float angle)
{
    rotateLocal(angle, smVec3f::UnitZ());
}

void smCamera::rotateFocusX(float angle)
{
    rotateFocus(angle, smVec3f::UnitX());
}

void smCamera::rotateFocusY(float angle)
{
    rotateFocus(angle, smVec3f::UnitY());
}

void smCamera::rotateFocusZ(float angle)
{
    rotateFocus(angle, smVec3f::UnitZ());
}

void smCamera::rotateGlobalX(float angle)
{
    rotateGlobal(angle, smVec3f::UnitX());
}

void smCamera::rotateGlobalY(float angle)
{
    rotateGlobal(angle, smVec3f::UnitY());
}

void smCamera::rotateGlobalZ(float angle)
{
    rotateGlobal(angle, smVec3f::UnitZ());
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
    this->view = smCamera::lookAt(pos, fp, getUpVec());
    viewDirty = false;
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
    projDirty = false;
}
