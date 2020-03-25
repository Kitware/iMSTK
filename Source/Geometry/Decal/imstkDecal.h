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

#pragma warning( push )
#pragma warning( disable : 4201 )
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#pragma warning( pop )

#include "imstkMath.h"
#include "imstkAnalyticalGeometry.h"

namespace imstk
{
class Decal : public AnalyticalGeometry
{
public:
    Decal(const std::string name = std::string(""));

    ///
    /// \brief Print the cube info
    ///
    void print() const override;

    ///
    /// \brief Returns the volume of the cube
    ///
    double getVolume() const override;

    ///
    /// \brief Update decal transforms
    /// \todo clear warning C4100
    ///
    void updateDecal(glm::mat4& viewMatrix);

protected:
    friend class VulkanDecalRenderDelegate;

    void applyScaling(const double s) override;

    glm::vec3 m_dimensions;

    glm::mat4 m_transform;
    glm::mat4 m_inverse;
};
}
