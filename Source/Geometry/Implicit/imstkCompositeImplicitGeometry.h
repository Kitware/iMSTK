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
/// \class ImplicitGeometry
///
/// \brief Class that can represent the geometry of multiple implicit geometries as boolean functions
/// One may supply it with multiple implicit geometries and the operation they would like
///
class CompositeImplicitGeometry : public ImplicitGeometry
{
public:
    enum class GeometryBoolType
    {
        Intersection,
        Difference,
        Union
    };

    using GeometryBoolPair = std::pair<std::shared_ptr<ImplicitGeometry>, GeometryBoolType>;

public:
    CompositeImplicitGeometry(std::string name    = "") : ImplicitGeometry(Type::CompositeImplicitGeometry, name) { }
    virtual ~CompositeImplicitGeometry() override = default;

public:
    ///
    /// \brief Returns signed distance to surface at pos
    ///
    virtual double getFunctionValue(const Vec3d& pos) const override;

    void addImplicitGeometry(std::shared_ptr<ImplicitGeometry> geometry, const GeometryBoolType type) { geometries.push_back(GeometryBoolPair(geometry, type)); }

protected:
    std::list<GeometryBoolPair> geometries;
};
}
