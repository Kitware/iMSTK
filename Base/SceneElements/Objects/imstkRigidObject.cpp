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

#include "imstkRigidObject.h"

namespace imstk
{

void
RigidObject::translate(const Vec3d& t)
{
    m_physicsGeometry->translate(t);
}

void
RigidObject::translate(const double& x, const double& y, const double& z)
{
    m_physicsGeometry->translate(Vec3d(x, y, z));
}

void
RigidObject::rotate(const Quatd& r)
{
    m_physicsGeometry->rotate(r);
}

void
RigidObject::rotate(const Mat3d& r)
{
    m_physicsGeometry->rotate(Quatd(r));
}

void
RigidObject::rotate(const Vec3d& axis, const double& angle)
{
    m_physicsGeometry->rotate(Quatd(Rotd(angle, axis)));
}

} // imstk
