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

const LightType
Light::getType()
{
    if( m_vtkLight->LightTypeIsSceneLight() )
    {
        return LightType::SCENE_LIGHT;
    }

    return LightType::HEAD_LIGHT;
}

void
Light::setType(const LightType& type)
{
    if( type == LightType::SCENE_LIGHT )
    {
        m_vtkLight->SetLightTypeToSceneLight();
    }
    else
    {
        m_vtkLight->SetLightTypeToHeadlight();
    }
}

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
    this->setPosition(p[0], p[1], p[2]);
}

void
Light::setPosition(const double& x,
                   const double& y,
                   const double& z)
{
    this->warningIfHeadLight();
    m_vtkLight->SetPosition(x, y, z);
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
    this->warningIfHeadLight();
    m_vtkLight->SetFocalPoint(x, y, z);
}

const bool
Light::isOn()
{
    return m_vtkLight->GetSwitch();
}

void
Light::switchOn()
{
    m_vtkLight->SwitchOn();
}

const bool
Light::isOff()
{
    return !this->isOn();
}

void
Light::switchOff()
{
    m_vtkLight->SwitchOff();
}

const bool
Light::isPositional()
{
    return m_vtkLight->GetPositional();
}

void
Light::setPositional()
{
    m_vtkLight->PositionalOn();
}

const bool
Light::isDirectional()
{
    return !this->isPositional();
}

void
Light::setDirectional()
{
    m_vtkLight->PositionalOff();
}

const double
Light::getSpotAngle() const
{
    return m_vtkLight->GetConeAngle();
}

void
Light::setSpotAngle(const double& angle)
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
    m_vtkLight->SetDiffuseColor(c(0), c(1), c(2));
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
Light::setName(std::string name)
{
    m_name = name;
}

void
Light::warningIfHeadLight()
{
    if( this->getType() == LightType::HEAD_LIGHT )
    {
        LOG(WARNING) << "Can not change position or focal point "
                     << "for a HEAD_LIGHT (linked to the active camera).\n"
                     << "Set your light type to SCENE_LIGHT to manually "
                     << "edit those parameters.";
    }
}
}
