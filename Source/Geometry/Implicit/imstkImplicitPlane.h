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

#include "imstkImplicitGeometry.h"

namespace imstk
{
///
/// \class ImplicitPlane
///
/// \brief Implicitly defined plane
///
class ImplicitPlane : public ImplicitGeometry
{
public:
    ImplicitPlane(const Vec3d& pos, const Vec3d& normal, std::string name = "") : ImplicitGeometry(Type::ImplicitPlane, name),
        m_pos(pos), m_normal(normal.normalized())
    {
    }

    virtual ~ImplicitPlane() override = default;

public:
    ///
    /// \brief Returns signed distance to surface at pos
    ///
    virtual double getFunctionValue(const Vec3d& pos) const override { return m_normal.dot(pos - m_pos); }

    ///
    /// \brief Returns gradient of signed distance field at pos
    ///
    virtual Vec3d getFunctionGrad(const Vec3d& pos) const override { return -getFunctionValue(pos) * m_normal; }

protected:
    // A point on the plane
    Vec3d m_pos;
    // The normal
    Vec3d m_normal;
};
} //imstk
