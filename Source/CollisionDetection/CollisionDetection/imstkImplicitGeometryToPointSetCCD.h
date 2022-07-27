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
template<typename T, int N> class VecDataArray;

class ImplicitGeometry;
class PointSet;

///
/// \class ImplicitGeometryToPointSetCCD
///
/// \brief ImplicitGeometry to PointSet continous collision detection.
/// This CD method marches along the displacement of the points in the pointset
/// to converge on the zero crossing of the implicit geometry. This particular
/// version is suited for levelsets not SDFs as it caches the history of the contact
/// to avoid sampling the implicit geometry anywhere but at the surface (it will also
/// work for SDFs, though better alterations/modifications of this exist for SDFs)
///
class ImplicitGeometryToPointSetCCD : public CollisionDetectionAlgorithm
{
public:
    ImplicitGeometryToPointSetCCD();
    ~ImplicitGeometryToPointSetCCD() override = default;

    IMSTK_TYPE_NAME(ImplicitGeometryToPointSetCCD)

protected:
    void setupFunctions(std::shared_ptr<ImplicitGeometry> implicitGeom, std::shared_ptr<PointSet> pointSet);

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

    std::shared_ptr<VecDataArray<double, 3>> m_displacementsPtr;

    std::unordered_map<int, Vec3d> m_prevOuterElement;
    std::unordered_map<int, int>   m_prevOuterElementCounter;

    // Penetration depths are clamped to this ratio * displacement of the vertex
    double m_depthRatioLimit = 0.3;
};
} // namespace imstk