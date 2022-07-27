/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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