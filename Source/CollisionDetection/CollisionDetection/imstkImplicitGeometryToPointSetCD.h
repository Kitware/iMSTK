/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkImplicitFunctionFiniteDifferenceFunctor.h"
#include "imstkMacros.h"

namespace imstk
{
///
/// \class ImplicitGeometryToPointSetCD
///
/// \brief ImplicitGeometry to PointSet collision detection. This generates PointDirection
/// collision data via signed distance sampling and central finite differences
///
class ImplicitGeometryToPointSetCD : public CollisionDetectionAlgorithm
{
public:
    ImplicitGeometryToPointSetCD();
    ~ImplicitGeometryToPointSetCD() override = default;

    IMSTK_TYPE_NAME(ImplicitGeometryToPointSetCD)

protected:
    ///
    /// \brief Compute collision data for AB simultaneously
    ///
    void computeCollisionDataAB(
        std::shared_ptr<Geometry>      geomA,
        std::shared_ptr<Geometry>      geomB,
        std::vector<CollisionElement>& elementsA,
        std::vector<CollisionElement>& elementsB) override;

    ///
    /// \brief Compute collision data for side A
    ///
    void computeCollisionDataA(
        std::shared_ptr<Geometry>      geomA,
        std::shared_ptr<Geometry>      geomB,
        std::vector<CollisionElement>& elementsA) override;

    ///
    /// \brief Compute collision data for side B
    ///
    void computeCollisionDataB(
        std::shared_ptr<Geometry>      geomA,
        std::shared_ptr<Geometry>      geomB,
        std::vector<CollisionElement>& elementsB) override;

private:
    ImplicitFunctionCentralGradient m_centralGrad;
};
} // namespace imstk