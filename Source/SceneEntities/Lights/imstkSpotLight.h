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

#include "imstkMacros.h"
#include "imstkPointLight.h"

namespace imstk
{
///
/// \class Spot light class
///
/// \brief A spot light is a point light in a cone shape.
///
class SpotLight : public PointLight
{
public:
    SpotLight() : m_spotAngle(10.0) { }
    ~SpotLight() override = default;

    IMSTK_TYPE_NAME(SpotLight)

    ///
    /// \brief Get the spotlight angle in degrees
    ///
    double getSpotAngle() const { return m_spotAngle; }

    ///
    /// \brief Set the spotlight angle in degrees
    ///
    void setSpotAngle(const double angle) { m_spotAngle = angle; }

protected:
    double m_spotAngle = 45.0;
};
} // namespace imstk