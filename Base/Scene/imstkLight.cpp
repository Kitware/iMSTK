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

#include "imstkLight.h"

#include <g3log/g3log.hpp>

namespace imstk {
const Vec3d
Light::getPosition() const
{
    double p[3];
    m_vtkLight->GetPosition(p);
    return Vec3d(p[0], p[1], p[2]);
}

void
Light::setPosition(const Vec3d& p)
{
    m_vtkLight->SetPosition(p[0], p[1], p[2]);
}

void
Light::setPosition(const double& x,
                   const double& y,
                   const double& z)
{
    m_vtkLight->SetPosition(x, y, z);
}

const Color
Light::getColor() const
{
    Color c;
    m_vtkLight->GetDiffuseColor(c.rgba);
    return c;
}

void
Light::setColor(const Color& c)
{
    m_vtkLight->SetColor(c(0), c(1), c(2));
}

const std::string&
Light::getName() const
{
    return m_name;
}

void
Light::setName(std::string name)
{
    m_name = name;
}
}
