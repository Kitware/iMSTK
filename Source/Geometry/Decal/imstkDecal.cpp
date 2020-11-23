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
#include "imstkLogger.h"

namespace imstk
{
Decal::Decal(const std::string& name) : m_dimensions(Vec3d(1.0, 1.0, 1.0)), AnalyticalGeometry(Geometry::Type::Decal, name)
{
}

void
Decal::print() const
{
    Geometry::print();
    LOG(INFO) << "Dimensions: " << m_dimensions[0]
              << ", " << m_dimensions[1]
              << ", " << m_dimensions[2];
}

void
Decal::applyScaling(const double s)
{
    m_dimensions *= s;
}

void
Decal::updateDecal(Mat4d& imstkNotUsed(viewMatrix))
{
    Mat4d transform = mat4dScale(Vec3d(getScaling(), getScaling(), getScaling()));
    transform = mat4dRotation(getRotation()) * transform * mat4dTranslate(getPosition());

    m_inverse = transform.inverse();
}
}