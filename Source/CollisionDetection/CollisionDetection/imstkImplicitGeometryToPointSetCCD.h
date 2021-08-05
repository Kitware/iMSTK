/*=========================================================================

Library: iMSTK

Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
& Imaging in Medicine, Rensselaer Polytechnic Institute.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0.txt

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

=========================================================================*/

#pragma once

#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkImplicitFunctionFiniteDifferenceFunctor.h"

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
    virtual ~ImplicitGeometryToPointSetCCD() override = default;

    ///
    /// \brief Returns collision detection type string name
    ///
    virtual const std::string getTypeName() const override { return "ImplicitGeometryToPointSetCCD"; }

protected:
    void setupFunctions(std::shared_ptr<ImplicitGeometry> implicitGeom, std::shared_ptr<PointSet> pointSet);

    ///
    /// \brief Compute collision data for AB simulatenously
    ///
    virtual void computeCollisionDataAB(
        std::shared_ptr<Geometry>          geomA,
        std::shared_ptr<Geometry>          geomB,
        CDElementVector<CollisionElement>& elementsA,
        CDElementVector<CollisionElement>& elementsB) override;

    ///
    /// \brief Compute collision data for side A
    ///
    virtual void computeCollisionDataA(
        std::shared_ptr<Geometry> geomA,
        std::shared_ptr<Geometry> geomB, CDElementVector<CollisionElement>& elementsA) override;

    ///
    /// \brief Compute collision data for side B
    ///
    virtual void computeCollisionDataB(
        std::shared_ptr<Geometry>          geomA,
        std::shared_ptr<Geometry>          geomB,
        CDElementVector<CollisionElement>& elementsB) override;

private:
    ImplicitFunctionCentralGradient m_centralGrad;

    std::shared_ptr<VecDataArray<double, 3>> m_displacementsPtr;

    std::unordered_map<int, Vec3d> m_prevOuterElement;
    std::unordered_map<int, int>   m_prevOuterElementCounter;

    // Penetration depths are clamped to this ratio * displacement of the vertex
    double m_depthRatioLimit = 0.3;
};
}