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
    SCENE_LIGHT,
    HEAD_LIGHT
};

///
/// \class Light
///
/// \brief Generic class for scene lights
///
class Light
{
public:
    ///
    /// \brief
    ///
    Light(std::string name) : m_name(name) {}

    ///
    /// \brief
    ///
    ~Light() = default;

    ///
    /// \brief
    ///
    const LightType getType();

    ///
    /// \brief
    ///
    void setType(const LightType& type);

    ///
    /// \brief
    ///
    const Vec3d getPosition() const;

    ///
    /// \brief
    ///
    void setPosition(const Vec3d& p);
    void setPosition(const double& x,
                     const double& y,
                     const double& z);

    ///
    /// \brief
    ///
    const Vec3d getFocalPoint() const;

    ///
    /// \brief
    ///
    void setFocalPoint(const Vec3d& p);
    void setFocalPoint(const double& x,
                     const double& y,
                     const double& z);

    ///
    /// \brief
    ///
    const bool isOn();

    ///
    /// \brief
    ///
    void switchOn();

    ///
    /// \brief
    ///
    const bool isOff();

    ///
    /// \brief
    ///
    void switchOff();

    ///
    /// \brief
    ///
    const bool isPositional();

    ///
    /// \brief
    ///
    void setPositional();

    ///
    /// \brief
    ///
    const bool isDirectional();

    ///
    /// \brief
    ///
    void setDirectional();

    ///
    /// \brief
    ///
    const double getSpotAngle() const;

    ///
    /// \brief
    ///
    void setSpotAngle(const double& angle);

    ///
    /// \brief
    ///
    const Color getColor() const;

    ///
    /// \brief
    ///
    void setColor(const Color& c);

    ///
    /// \brief
    ///
    vtkSmartPointer<vtkLight> getVtkLight() const;

    ///
    /// \brief
    ///
    const std::string& getName() const;

    ///
    /// \brief
    ///
    void setName(std::string name);

protected:
    ///
    /// \brief
    ///
    void warningIfHeadLight();

    vtkSmartPointer<vtkLight> m_vtkLight = vtkSmartPointer<vtkLight>::New();
    std::string m_name;
};

} // imstk

#endif // ifndef imstkLight_h
