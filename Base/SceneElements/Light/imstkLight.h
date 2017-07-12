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
/// \brief Abstract class for lights
///
class Light
{
public:
    ///
    /// \brief
    ///
    LightType getType();

    ///
    /// \brief
    ///
    void setType(const LightType& type);

    ///
    /// \brief Get the light focal point
    ///
    const Vec3d getFocalPoint() const;

    ///
    /// \brief Set the light focal point
    ///
    void setFocalPoint(const Vec3d& p);
    void setFocalPoint(const double& x,
                       const double& y,
                       const double& z);

    ///
    /// \brief
    ///
    bool isOn();

    ///
    /// \brief
    ///
    void switchOn();

    ///
    /// \brief
    ///
    bool isOff();

    ///
    /// \brief
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
    void setName(std::string name);

protected:
    Light(std::string name)
    {
        m_name = name;
    };

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
    DirectionalLight(std::string name) : Light(name)
    {
        m_type = LightType::DIRECTIONAL_LIGHT;
        m_vtkLight->SetPositional(false);
    };
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
    PointLight(std::string name) : Light(name)
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
    SpotLight(std::string name) : PointLight(name)
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
