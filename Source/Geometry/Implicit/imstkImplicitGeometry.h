/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkGeometry.h"

namespace imstk
{
///
/// \class ImplicitGeometry
///
/// \brief Defines implicit geometry with an implicit function
/// Implicit functions must be decoupled from geometry
///
class ImplicitGeometry : public Geometry
{
public:
    ~ImplicitGeometry() override = default;

    ///
    /// \brief Returns function value given position
    ///
    virtual double getFunctionValue(const Vec3d& pos) const = 0;
};
} // namespace imstk
