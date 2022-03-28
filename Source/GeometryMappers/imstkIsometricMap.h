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
/// \class IsometricMap
///
/// \brief A maps that lets the child follow the parent transform
///
class IsometricMap : public GeometryMap
{
public:
    IsometricMap() = default;
    IsometricMap(std::shared_ptr<Geometry> parent,
        std::shared_ptr<Geometry> child)
    {
        this->setParentGeometry(parent);
        this->setChildGeometry(child);
    }
    ~IsometricMap() override = default;

    IMSTK_TYPE_NAME(IsometricMap)

    ///
    /// \brief Compute the map
    ///
    void compute() override { }

protected:
    ///
    /// \brief Apply the map
    ///
    void requestUpdate() override;
};
} // namespace imstk