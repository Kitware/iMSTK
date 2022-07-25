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
/// \class SurfaceMeshToSphereCD
///
/// \brief SurfaceMesh to Sphere collision detection
/// Generates vertex-triangle, point-edge, and point-point CD data
/// By default only generates contact data for the pointset.
///
class SurfaceMeshToSphereCD : public CollisionDetectionAlgorithm
{
public:
    SurfaceMeshToSphereCD();
    ~SurfaceMeshToSphereCD() override = default;

    IMSTK_TYPE_NAME(SurfaceMeshToSphereCD)

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
