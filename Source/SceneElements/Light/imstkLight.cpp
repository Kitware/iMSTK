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

namespace imstk
{
LightType
Light::getType() const
{
    return m_type;
}

void
Light::setType(const LightType& type)
{
    m_type = type;
}

const Vec3d
PointLight::getPosition() const
{
    double p[3];
    m_vtkLight->GetPosition(p);
    return Vec3d(p[0], p[1], p[2]);
}

void
PointLight::setPosition(const Vec3d& p)
{
    this->setPosition(p[0], p[1], p[2]);
}

void
PointLight::setPosition(const double& x,
                        const double& y,
                        const double& z)
{
    m_vtkLight->SetPosition(x, y, z);
}

void
DirectionalLight::setFocalPoint(const Vec3d& p)
{
    this->setFocalPoint(p[0], p[1], p[2]);
}

void
DirectionalLight::setFocalPoint(const double& x,
                                const double& y,
                                const double& z)
{
    if (x != 0 || y != 0 || z != 0)
    {
        m_vtkLight->SetFocalPoint(x, y, z);
    }
    else
    {
        LOG(WARNING) << "Directional lights can't have focal point at (0, 0, 0)";
    }
}

const Vec3d
Light::getFocalPoint() const
{
    double p[3];
    m_vtkLight->GetFocalPoint(p);
    return Vec3d(p[0], p[1], p[2]);
}

void
Light::setFocalPoint(const Vec3d& p)
{
    this->setFocalPoint(p[0], p[1], p[2]);
}

void
Light::setFocalPoint(const double& x,
                     const double& y,
                     const double& z)
{
    m_vtkLight->SetFocalPoint(x, y, z);
}

bool
Light::isOn() const
{
    return m_vtkLight->GetSwitch() != 0;
}

void
Light::switchOn() const
{
    m_vtkLight->SwitchOn();
}

bool
Light::isOff() const
{
    return !this->isOn();
}

void
Light::switchOff()
{
    m_vtkLight->SwitchOff();
}

double
SpotLight::getSpotAngle() const
{
    return m_vtkLight->GetConeAngle();
}

void
SpotLight::setSpotAngle(const double& angle)
{
    m_vtkLight->SetConeAngle(angle);
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

float
Light::getIntensity() const
{
    return m_vtkLight->GetIntensity();
}

void
Light::setIntensity(const float intensity)
{
    m_vtkLight->SetIntensity(intensity);
}

vtkSmartPointer<vtkLight>
Light::getVtkLight() const
{
    return m_vtkLight;
}

const std::string&
Light::getName() const
{
    return m_name;
}

void
Light::setName(const std::string& name)
{
    m_name = name;
}

void
Light::setName(const std::string&& name)
{
    m_name = std::move(name);
}
} // imstk
