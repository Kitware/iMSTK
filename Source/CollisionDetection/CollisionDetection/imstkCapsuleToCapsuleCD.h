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
/// \class CapsuleToCapsuleCD
///
/// \brief Capsule to Capsule collision detection.
///
/// To compute intersection between two capsules first find the nearest
/// point on the edges/centerlines of the capsules. Then perform sphere to
/// sphere intersection between two spheres of capsules radius at these
/// two points.
///
class CapsuleToCapsuleCD : public CollisionDetectionAlgorithm
{
public:
    CapsuleToCapsuleCD();
    ~CapsuleToCapsuleCD() override = default;

    IMSTK_TYPE_NAME(CapsuleToCapsuleCD)

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