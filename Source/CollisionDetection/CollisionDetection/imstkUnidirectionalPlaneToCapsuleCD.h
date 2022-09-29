/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkMacros.h"

namespace imstk
{
///
/// \class UnidirectionalPlaneToCapsuleCD
///
/// \brief Plane to capsule collision detection
/// Generates point-direction contact data.
/// By default only generates contact data for the sphere
///
class UnidirectionalPlaneToCapsuleCD : public CollisionDetectionAlgorithm
{
public:
    UnidirectionalPlaneToCapsuleCD();
    ~UnidirectionalPlaneToCapsuleCD() override = default;

    IMSTK_TYPE_NAME(UnidirectionalPlaneToCapsuleCD)

protected:
    ///
    /// \brief Compute collision data for AB simultaneously
    ///
    void computeCollisionDataAB(
        std::shared_ptr<Geometry>      geomA,
        std::shared_ptr<Geometry>      geomB,
        std::vector<CollisionElement>& elementsA,
        std::vector<CollisionElement>& elementsB) override;
};
} // namespace imstk