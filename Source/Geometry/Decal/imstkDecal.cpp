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
Decal::Decal(const std::string& name) : AnalyticalGeometry(Geometry::Type::Decal, name),
    m_dimensions(Vec3d(1.0, 1.0, 1.0))
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
Decal::applyTransform(const Mat4d& m)
{
    const Vec3d s = Vec3d(
        m.block<3, 1>(0, 0).norm(),
        m.block<3, 1>(0, 1).norm(),
        m.block<3, 1>(0, 2).norm());
    m_dimensions = m_dimensions.cwiseProduct(s);
}

void
Decal::updateDecal(Mat4d& imstkNotUsed(viewMatrix))
{
    m_inverse = m_transform.inverse();
}
}