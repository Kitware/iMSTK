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

#include "imstkGeometryMap.h"
#include "imstkMacros.h"

namespace imstk
{
///
/// \class IdentityMap
///
/// \brief A maps that lets the child follow the parent's position and orientation
///
class IdentityMap : public GeometryMap
{
public:
    IdentityMap() { }
    ~IdentityMap() override = default;

    IMSTK_TYPE_NAME(IdentityMap)

    ///
    /// \brief Compute the map
    ///
    void compute() override {}

    ///
    /// \brief Apply the map
    ///
    void apply() override;

    ///
    /// \brief Check the validity of the map
    ///
    bool isValid() const override { return true; }

    // Accessors

    ///
    /// \brief DISABLED: Set the transform of the Identity map
    ///
    void setTransform(const RigidTransform3d& affineTransform) = delete;

    ///
    /// \brief Get the transform of Identity map which is an Identity (3x3)
    ///
    const RigidTransform3d getTransform() const;
};
} // namespace imstk