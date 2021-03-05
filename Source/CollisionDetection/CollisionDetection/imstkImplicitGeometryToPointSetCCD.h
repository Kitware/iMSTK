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

#include "imstkCollisionDetection.h"
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
class ImplicitGeometryToPointSetCCD : public CollisionDetection
{
public:
    ///
    /// \brief
    /// \param ImplicitGeometry
    /// \param PointSet to test collision with
    /// \param CollisionData to write too
    ///
    ImplicitGeometryToPointSetCCD(std::shared_ptr<ImplicitGeometry> implicitGeomA,
                                  std::shared_ptr<PointSet>         pointSetB,
                                  std::shared_ptr<CollisionData>    colData);
    virtual ~ImplicitGeometryToPointSetCCD() override = default;

public:
    ///
    /// \brief Detect collision and compute collision data
    ///
    void computeCollisionData() override;

private:
    std::shared_ptr<ImplicitGeometry> m_implicitGeomA;
    std::shared_ptr<PointSet>       m_pointSetB;
    ImplicitFunctionCentralGradient centralGrad;

    std::shared_ptr<VecDataArray<double, 3>> displacementsPtr;

    std::unordered_map<int, Vec3d> prevOuterElement;
    std::unordered_map<int, int>   prevOuterElementCounter;

    // Penetration depths are clamped to this ratio * displacement of the vertex
    double m_depthRatioLimit = 0.3;
};
}