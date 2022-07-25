/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkCollisionDetectionAlgorithm.h"

namespace imstk
{
class Geometry;

///
/// \class CCDAlgorithm
///
/// \brief Base class for all Continuous Collision Detection (CCD) classes.
/// A continuous collision detection algorithm typically requires geometries in two
/// timesteps (previous, current) for continuous analysis between the two.
/// CCDAlgorithm classes follow the same logic as CollisionDetectionAlgorithm.
/// This base class for CCD algorithms enforces the implementation of a function
/// (UpdatePreviousTimestepGeometry) to cache/store previous state of the colliding
/// geometries.
///
class CCDAlgorithm : public CollisionDetectionAlgorithm
{
public:
    ///
    /// \brief Call with finalized geometries to be cached for reference
    /// when computing continuous collision in a future iteration.
    ///
    virtual void updatePreviousTimestepGeometry(
        std::shared_ptr<const Geometry> geomA, std::shared_ptr<const Geometry> geomB) = 0;

protected:
    CCDAlgorithm() : CollisionDetectionAlgorithm() {}
};
} // namespace imstk
