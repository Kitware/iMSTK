/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkImplicitGeometry.h"
#include "imstkMacros.h"

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

    IMSTK_TYPE_NAME(CompositeImplicitGeometry)

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
