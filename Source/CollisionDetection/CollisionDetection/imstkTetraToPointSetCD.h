/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkMacros.h"
#include "imstkSpatialHashTableSeparateChaining.h"

namespace imstk
{
///
/// \class TetraToPointSetCD
///
/// \brief Computes if points lie in tetrahedrons using spatial hashing
/// Generates tetra-point contact data.
/// By default only generates contact data for both sides.
///
class TetraToPointSetCD : public CollisionDetectionAlgorithm
{
public:
    TetraToPointSetCD();
    ~TetraToPointSetCD() override = default;

    IMSTK_TYPE_NAME(TetraToPointSetCD)

public:
    ///
    /// \brief Compute collision data for both sides simultaneously
    ///
    void computeCollisionDataAB(
        std::shared_ptr<Geometry>      geomA,
        std::shared_ptr<Geometry>      geomB,
        std::vector<CollisionElement>& elementsA,
        std::vector<CollisionElement>& elementsB) override;

protected:
    SpatialHashTableSeparateChaining m_hashTableA; ///< Spatial hash table
    SpatialHashTableSeparateChaining m_hashTableB; ///< Spatial hash table
};
} // namespace imstk