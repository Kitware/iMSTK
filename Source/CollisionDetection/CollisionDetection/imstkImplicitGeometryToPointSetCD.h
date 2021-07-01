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
    virtual ~ImplicitGeometryToPointSetCD() override = default;

    ///
    /// \brief Returns collision detection type string name
    ///
    virtual const std::string getTypeName() const override { return "ImplicitGeometryToPointSetCD"; }

protected:
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
        std::shared_ptr<Geometry>          geomA,
        std::shared_ptr<Geometry>          geomB,
        CDElementVector<CollisionElement>& elementsA) override;

    ///
    /// \brief Compute collision data for side B
    ///
    virtual void computeCollisionDataB(
        std::shared_ptr<Geometry>          geomA,
        std::shared_ptr<Geometry>          geomB,
        CDElementVector<CollisionElement>& elementsB) override;

private:
    ImplicitFunctionCentralGradient m_centralGrad;
};
}