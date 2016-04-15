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

#include "imstkCamera.h"

#include <g3log/g3log.hpp>

namespace imstk {
const Vec3d
Camera::getPosition() const
{
    double p[3];
    m_vtkCamera->GetPosition(p);
    return Vec3d(p[0], p[1], p[2]);
}

void
Camera::setPosition(const Vec3d& p)
{
    this->setPosition(p[0], p[1], p[2]);
}

void
Camera::setPosition(const double& x,
                   const double& y,
                   const double& z)
{
    m_vtkCamera->SetPosition(x, y, z);
}

const Vec3d
Camera::getFocalPoint() const
{
    double p[3];
    m_vtkCamera->GetFocalPoint(p);
    return Vec3d(p[0], p[1], p[2]);
}

void
Camera::setFocalPoint(const Vec3d& p)
{
    this->setFocalPoint(p[0], p[1], p[2]);
}

void
Camera::setFocalPoint(const double& x,
                   const double& y,
                   const double& z)
{
    m_vtkCamera->SetFocalPoint(x, y, z);
}

const Vec3d
Camera::getViewUp() const
{
    double v[3];
    m_vtkCamera->GetViewUp(v);
    return Vec3d(v[0], v[1], v[2]);
}

void
Camera::setViewUp(const Vec3d& v)
{
    this->setViewUp(v[0], v[1], v[2]);
}

void
Camera::setViewUp(const double& x,
                   const double& y,
                   const double& z)
{
    m_vtkCamera->SetViewUp(x, y, z);
}

const double
Camera::getViewAngle() const
{
    return m_vtkCamera->GetViewAngle();
}

void
Camera::setViewAngle(const double& angle)
{
    m_vtkCamera->SetViewAngle(angle);
}

vtkSmartPointer<vtkCamera>
Camera::getVtkCamera() const
{
    return m_vtkCamera;
}
}
