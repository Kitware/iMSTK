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

protected:
    Light(const LightType& type) : SceneEntity(), m_type(type) { }

    // properties with defaults
    float m_intensity   = 1.;
    Color m_color       = Color::White;
    bool  m_switchState = true;
    Vec3f m_focalPoint  = Vec3f(0, 0, 0);

    LightType m_type;
};
} // imstk