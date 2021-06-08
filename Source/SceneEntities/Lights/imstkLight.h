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

#pragma once

#include "imstkMath.h"
#include "imstkColor.h"
#include "imstkSceneEntity.h"

#include <string>

namespace imstk
{
///
/// \brief Enumeration for the type of light
///
enum class LightType
{
    Directional,
    Point,
    Spot
};

///
/// \class Light
///
/// \brief Abstract base class for lights
///
class Light : public SceneEntity
{
public:
    virtual ~Light() override = default;

public:
    ///
    /// \brief Returns the type of light (see imstk::LightType)
    ///
    LightType getType() const { return m_type; }

    ///
    /// \brief Set the type of the light
    ///
    void setType(const LightType& type) { m_type = type; }

    ///
    /// \brief Set the light focal point
    ///
    virtual void setFocalPoint(const Vec3d& p) { this->setFocalPoint((float)p[0], (float)p[1], (float)p[2]); }
    virtual void setFocalPoint(const float& x, const float& y, const float& z);
    const Vec3f getFocalPoint() const { return m_focalPoint; }

    ///
    /// \brief Get the status (On/off) of the light
    ///
    bool isOn() const { return m_switchState; }

    ///
    /// \brief Switch the light On
    ///
    void switchOn() { m_switchState = true; }

    ///
    /// \brief Get the status (On/off) of the light
    ///
    bool isOff() const { return m_switchState; }

    ///
    /// \brief Switch the light Off
    ///
    void switchOff() { m_switchState = false; }

    ///
    /// \brief Get the light color
    ///
    const Color getColor() const { return m_color; }

    ///
    /// \brief Set the light color
    ///
    void setColor(const Color& c) { m_color = c; }

    ///
    /// \brief Get the light intensity
    ///
    float getIntensity() const { return m_intensity; }

    ///
    /// \brief Set the light intensity. This value is unbounded.
    ///
    void setIntensity(double intensity) { m_intensity = (float)intensity; }

    ///
    /// \brief Get the light name
    ///
    const std::string& getName() const { return m_name; }

    ///
    /// \brief Set the light name
    ///
    void setName(std::string name) { m_name = name; }

protected:
    Light(const std::string& name, const LightType& type) : SceneEntity(), m_name(name), m_type(type) { }

    // properties with defaults
    float m_intensity   = 1.;
    Color m_color       = Color::White;
    bool  m_switchState = true;
    Vec3f m_focalPoint  = Vec3f(0, 0, 0);

    std::string m_name;
    LightType   m_type;
};

//-----------------------------------------------------------------------------

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
    DirectionalLight(const std::string& name) : Light(name, LightType::Directional)
    {
        this->setFocalPoint(-1.0f, -1.0f, -1.0f);
    }

    virtual ~DirectionalLight() override = default;

public:
    virtual const std::string getTypeName() const { return "DirectionalLight"; }

    ///
    /// \brief Direction of the light
    ///
    void setDirection(const Vec3d& dir) { setFocalPoint(dir); }
    void setDirection(const double x, const double y, const double z) { setFocalPoint(Vec3d(x, y, z)); }

protected:
};

//-----------------------------------------------------------------------------

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
    PointLight(const std::string& name, const LightType& type = LightType::Point) : Light(name, type) { }

    virtual ~PointLight() override = default;

public:
    virtual const std::string getTypeName() const { return "PointLight"; }

    ///
    /// \brief Get the cone angle
    ///
    const float getConeAngle() const { return m_coneAngle; }

    ///
    /// \brief Get the light position
    ///
    void setConeAngle(const double angle) { m_coneAngle = (float)angle; }

    ///
    /// \brief Get the light position
    ///
    const Vec3f getPosition() const { return m_position; }

    ///
    /// \brief Set the light position
    ///
    void setPosition(const Vec3d& p)
    {
        m_position = Vec3f(
            static_cast<float>(p[0]),
            static_cast<float>(p[1]),
            static_cast<float>(p[2]));
    };
    void setPosition(const double& x, const double& y, const double& z)
    {
        this->setPosition(Vec3d(x, y, z));
    }

protected:
    Vec3f m_position  = Vec3f(0.0f, 0.0f, 0.0f);
    float m_coneAngle = 179.0f;
};

//-----------------------------------------------------------------------------

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
    SpotLight(const std::string& name) : PointLight(name, LightType::Spot)
    {
        m_coneAngle = 10.0f;
    }

    virtual ~SpotLight() override = default;

public:
    virtual const std::string getTypeName() const { return "SpotLight"; }

    ///
    /// \brief Get the spotlight angle in degrees
    ///
    float getSpotAngle() const { return m_spotAngle; }

    ///
    /// \brief Set the spotlight angle in degrees
    ///
    void setSpotAngle(const double& angle) { m_spotAngle = static_cast<float>(angle); }

protected:
    float m_spotAngle = 45.0f;
};
} // imstk