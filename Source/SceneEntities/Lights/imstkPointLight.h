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
/// \brief A point light has a position, and it's range is determined by it's
/// intensity.
///
class PointLight : public Light
{
public:
    PointLight() = default;
    ~PointLight() override = default;

    const std::string getTypeName() const override { return "PointLight"; }

    const Vec3d getPosition() const { return m_position; }
    void setPosition(const Vec3d& p) { m_position = p; }
    void setPosition(const double x, const double y, const double z)
    {
        this->setPosition(Vec3d(x, y, z));
    }

protected:
    Vec3d m_position = Vec3d::Zero();
};
}