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
/// \class LineMeshToCapsuleCD
///
/// \brief LineMesh to Capsule collision detection
/// Generates point-edge and point-point CD data
/// By default only generates contact data for the pointset.
///
class LineMeshToCapsuleCD : public CollisionDetectionAlgorithm
{
public:
    LineMeshToCapsuleCD();
    ~LineMeshToCapsuleCD() override = default;

    IMSTK_TYPE_NAME(LineMeshToCapsuleCD)

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