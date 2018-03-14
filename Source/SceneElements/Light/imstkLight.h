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

#ifndef imstkLight_h
#define imstkLight_h

#include <string>

// vtk
#include <vtkSmartPointer.h>
#include <vtkLight.h>

// imstk
#include "imstkMath.h"
#include "imstkColor.h"

namespace imstk
{
///
/// \brief Enumeration for the type of light
///
enum class LightType
{
    DIRECTIONAL_LIGHT,
    POINT_LIGHT,
    SPOT_LIGHT
};

///
/// \class Light
///
/// \brief Abstract base class for lights
///
class Light
{
public:
    ///
    /// \brief Returns the type of light (see imstk::LightType)
    ///
    LightType getType() const;

    ///
    /// \brief Set the type of the light
    ///
    void setType(const LightType& type);

    ///
    /// \brief Get the light focal point
    ///
    const Vec3d getFocalPoint() const;

    ///
    /// \brief Set the light focal point
    ///
    virtual void setFocalPoint(const Vec3d& p);
    virtual void setFocalPoint(const double& x,
                               const double& y,
                               const double& z);

    ///
    /// \brief Get the status (On/off) of the light
    ///
    bool isOn() const;

    ///
    /// \brief Switch the light On
    ///
    void switchOn() const;

    ///
    /// \brief Get the status (On/off) of the light
    ///
    bool isOff() const;

    ///
    /// \brief Switch the light Off
    ///
    void switchOff();

    ///
    /// \brief Get the light color
    ///
    const Color getColor() const;

    ///
    /// \brief Set the light color
    ///
    void setColor(const Color& c);

    ///
    /// \brief Get the light intensity
    ///
    float getIntensity() const;

    ///
    /// \brief Set the light intensity. This value is unbounded.
    ///
    void setIntensity(float intensity);

    ///
    /// \brief Get the VTK light
    ///
    vtkSmartPointer<vtkLight> getVtkLight() const;

    ///
    /// \brief Get the light name
    ///
    const std::string& getName() const;

    ///
    /// \brief Set the light name
    ///
    void setName(const std::string& name);
    void setName(const std::string&& name);

protected:
    Light(const std::string& name) : m_name(name){};
    Light(std::string&& name) : m_name(std::move(name)){};

    vtkSmartPointer<vtkLight> m_vtkLight = vtkSmartPointer<vtkLight>::New();
    std::string m_name;
    LightType m_type;
};

///
/// \class DirectionalLight
///
/// \brief Directional light class
///
/// A directional light has no position or range. The focal point is the
/// direction.
///
class DirectionalLight : public Light
{
public:
    ///
    /// \brief Constructor
    ///
    DirectionalLight(const std::string& name) : Light(name)
    {
        m_type = LightType::DIRECTIONAL_LIGHT;
        m_vtkLight->SetPositional(false);
        this->setFocalPoint(-1, -1, -1);
    };
    DirectionalLight(std::string&& name) : Light(std::move(name))
    {
        m_type = LightType::DIRECTIONAL_LIGHT;
        m_vtkLight->SetPositional(false);
        this->setFocalPoint(-1, -1, -1);
    };

    virtual void setFocalPoint(const Vec3d& p);

    virtual void setFocalPoint(const double& x,
                               const double& y,
                               const double& z);

    ///
    /// \brief Turn shadows on
    ///
    void setCastsShadow(bool shadow);

    ///
    /// \brief Center point for shadow projection
    /// Sets the shadow map center to this position
    ///
    void setShadowCenter(Vec3f position);

    ///
    /// \brief Range for shadows
    /// A smaller range results in a denser shadow map
    ///
    void setShadowRange(float range);

protected:
    friend class VulkanRenderer;

    bool m_castShadow = true;
    Vec3f m_shadowCenter = Vec3f(0, 0, 0);
    float m_shadowRange = 2.0;
    int m_shadowMapIndex = -1;
};

///
/// \class PointLight
///
/// \brief Point light class
///
/// A point light has a position, and it's range is determined by it's
/// intensity.
///
class PointLight : public Light
{
public:
    ///
    /// \brief Constructors
    ///
    PointLight(const std::string& name) : Light(name)
    {
        m_type = LightType::POINT_LIGHT;
        m_vtkLight->SetPositional(true);
        m_vtkLight->SetConeAngle(179.0);
    };
    PointLight(std::string&& name) : Light(std::move(name))
    {
        m_type = LightType::POINT_LIGHT;
        m_vtkLight->SetPositional(true);
        m_vtkLight->SetConeAngle(179.0);
    };

    ///
    /// \brief Get the light position
    ///
    const Vec3d getPosition() const;

    ///
    /// \brief Set the light position
    ///
    void setPosition(const Vec3d& p);
    void setPosition(const double& x,
                     const double& y,
                     const double& z);
};

///
/// \class Spot light class
///
/// \brief A spot light is a point light in a cone shape.
///
class SpotLight : public PointLight
{
public:
    ///
    /// \brief Constructors
    ///
    SpotLight(const std::string& name) : PointLight(name)
    {
        m_type = LightType::SPOT_LIGHT;
        m_vtkLight->SetConeAngle(45);
    };
    SpotLight(std::string&& name) : PointLight(std::move(name))
    {
        m_type = LightType::SPOT_LIGHT;
        m_vtkLight->SetConeAngle(45);
    };

    ///
    /// \brief Get the spotlight angle in degrees
    ///
    double getSpotAngle() const;

    ///
    /// \brief Set the spotlight angle in degrees
    ///
    void setSpotAngle(const double& angle);
};
} // imstk

#endif // ifndef imstkLight_h
