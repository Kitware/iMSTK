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

#include "imstkLight.h"

namespace imstk
{
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
} // imstk