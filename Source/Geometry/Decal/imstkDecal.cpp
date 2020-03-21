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

#include "imstkDecal.h"

namespace imstk
{
Decal::Decal(const std::string name) : AnalyticalGeometry(Geometry::Type::Decal, name)
{
    m_dimensions = glm::vec3(1);
}

void
Decal::print() const
{
    Geometry::print();
    LOG(INFO) << "Dimensions: " << m_dimensions.x
              << ", " << m_dimensions.y
              << ", " << m_dimensions.z;
}

double
Decal::getVolume() const
{
    return (double)m_dimensions.x * (double)m_dimensions.y * (double)m_dimensions.z;
}

void
Decal::applyScaling(const double s)
{
    m_dimensions *= s;
}

#pragma warning(push)
#pragma warning( disable : 4100 )
void
Decal::updateDecal(glm::mat4& viewMatrix)
#pragma warning(pop)
{
    glm::mat4 transform;

    glm::vec3 scale((float) this->getScaling());
    transform = glm::scale(transform, scale);

    auto      rotation = this->getRotation();
    glm::mat3 rotationMatrix(rotation(0, 0), rotation(0, 1), rotation(0, 2),
                             rotation(1, 0), rotation(1, 1), rotation(1, 2),
                             rotation(2, 0), rotation(2, 1), rotation(2, 2));

    transform = glm::mat4(rotationMatrix) * transform;

    transform[3][0] = (float)this->getPosition().x();
    transform[3][1] = (float)this->getPosition().y();
    transform[3][2] = (float)this->getPosition().z();

    m_transform = transform;

    m_inverse = glm::inverse(m_transform);
}
}
