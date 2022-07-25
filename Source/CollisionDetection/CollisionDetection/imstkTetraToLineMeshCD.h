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
/// \class TetraToLineMeshCD
///
/// \brief Computes intersection points along a line mesh on the faces of the tetrahedrons
///
class TetraToLineMeshCD : public CollisionDetectionAlgorithm
{
public:
    TetraToLineMeshCD();
    ~TetraToLineMeshCD() override = default;

    IMSTK_TYPE_NAME(TetraToLineMeshCD)

public:
    ///
    /// \brief Compute collision data for both sides simultaneously
    ///
    void computeCollisionDataAB(
        std::shared_ptr<Geometry>      geomA,
        std::shared_ptr<Geometry>      geomB,
        std::vector<CollisionElement>& elementsA,
        std::vector<CollisionElement>& elementsB) override;
};
} // namespace imstk
