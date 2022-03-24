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

    CompositeImplicitGeometry() = default;
    ~CompositeImplicitGeometry() override = default;

    ///
    /// \brief Returns the string representing the type name of the geometry
    ///
    const std::string getTypeName() const override { return "CompositeImplicitGeometry"; }

    ///
    /// \brief Returns signed distance to surface at pos
    ///
    double getFunctionValue(const Vec3d& pos) const override;

    ///
    /// \brief Add an implicit geometry to have boolean operations done with
    /// \param geometry to use
    /// \param boolean function type to use
    ///
    void addImplicitGeometry(std::shared_ptr<ImplicitGeometry> geometry, const GeometryBoolType type)
    {
        geometry->updatePostTransformData();
        geometries.push_back(GeometryBoolPair(geometry, type));
    }

protected:
    std::list<GeometryBoolPair> geometries;
};
} // namespace imstk
