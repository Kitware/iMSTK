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

#include <vtkSmartPointer.h>
#include <vtkLight.h>

#include "imstkMath.h"
#include "imstkColor.h"

namespace imstk
{

enum class LightType
{
    SCENE_LIGHT,
    HEAD_LIGHT
};

class Light
{
public:

    Light(std::string name) : m_name(name) {}

    ~Light() = default;

    const LightType getType();
    void setType(const LightType& type);



    const Vec3d getPosition() const;
    void setPosition(const Vec3d& p);
    void setPosition(const double& x,
                     const double& y,
                     const double& z);

    const Vec3d getFocalPoint() const;
    void setFocalPoint(const Vec3d& p);
    void setFocalPoint(const double& x,
                     const double& y,
                     const double& z);

    const bool isOn();
    void switchOn();

    const bool isOff();
    void switchOff();

    const bool isPositional();
    void setPositional();

    const bool isDirectional();
    void setDirectional();

    const double getSpotAngle() const;
    void setSpotAngle(const double& angle);

    const Color getColor() const;
    void setColor(const Color& c);

    vtkSmartPointer<vtkLight> getVtkLight() const;

    const std::string& getName() const;
    void setName(std::string name);

protected:

    void warningIfHeadLight();

    vtkSmartPointer<vtkLight> m_vtkLight = vtkSmartPointer<vtkLight>::New();
    std::string m_name;

};
}

#endif // ifndef imstkLight_h
