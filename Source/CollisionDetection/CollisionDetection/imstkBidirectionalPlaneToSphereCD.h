/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkCollisionDetectionAlgorithm.h"

namespace imstk
{
///
/// \class BidirectionalPlaneToSphereCD
///
/// \brief Plane to sphere collision detection
/// Generates point-direction contact data.
///
class BidirectionalPlaneToSphereCD : public CollisionDetectionAlgorithm
{
public:
    BidirectionalPlaneToSphereCD();
    ~BidirectionalPlaneToSphereCD() override = default;

    IMSTK_TYPE_NAME(BidirectionalPlaneToSphereCD)

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