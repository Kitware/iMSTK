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
class CollisionData;
class Sphere;

///
/// \class SphereToSphereCD
///
/// \brief Sphere to sphere collision detection
/// Generates point-direction contact data.
/// By default generates contact data for both sides.
///
class SphereToSphereCD : public CollisionDetectionAlgorithm
{
public:
    SphereToSphereCD();
    ~SphereToSphereCD() override = default;

    IMSTK_TYPE_NAME(SphereToSphereCD)

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