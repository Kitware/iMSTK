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
    DirectionalLight()
    {
        this->setFocalPoint(-1.0, -1.0, -1.0);
    }

    virtual ~DirectionalLight() override = default;

public:
    const std::string getTypeName() const override { return "DirectionalLight"; }

    ///
    /// \brief Direction of the light
    ///
    void setDirection(const Vec3d& dir) { setFocalPoint(dir); }
    void setDirection(const double x, const double y, const double z) { setFocalPoint(Vec3d(x, y, z)); }
};
} // imstk