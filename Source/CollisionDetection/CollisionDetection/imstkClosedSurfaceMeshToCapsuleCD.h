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
/// \class ClosedSurfaceMeshToCapsuleCD
///
/// \brief Closed SurfaceMesh to Capsule collision detection
/// Generates point-triangle
///
class ClosedSurfaceMeshToCapsuleCD : public CollisionDetectionAlgorithm
{
public:
    ClosedSurfaceMeshToCapsuleCD();
    ~ClosedSurfaceMeshToCapsuleCD() override = default;

    IMSTK_TYPE_NAME(ClosedSurfaceMeshToCapsuleCD)

    void setRadiusMultiplier(double mult) { m_radiusMultiplier = mult; }
    double getRadiusMultiplier() { return m_radiusMultiplier; }

protected:
    ///
    /// \brief Compute collision data for AB simultaneously
    ///
    void computeCollisionDataAB(
        std::shared_ptr<Geometry>      geomA,
        std::shared_ptr<Geometry>      geomB,
        std::vector<CollisionElement>& elementsA,
        std::vector<CollisionElement>& elementsB) override;

    double m_radiusMultiplier = 8.0; // Multiplier used to filter triangles
};
} // namespace imstk
