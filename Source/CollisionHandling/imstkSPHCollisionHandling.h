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

#include "imstkCollisionHandling.h"

namespace imstk
{
class CollisionData;
class ImplicitGeometryToPointSetCD;
class SPHObject;

///
/// \class SPHCollisionHandling
///
/// \brief The SPHCollisionHandler consumes PointIndexDirection contact data
/// to resolve positions and correct velocities of SPH particles. It does
/// not correct pressures/densities.
///
class SPHCollisionHandling : public CollisionHandling
{
public:
    SPHCollisionHandling() = default;
    virtual ~SPHCollisionHandling() override = default;

    virtual const std::string getTypeName() const override { return "SPHCollisionHandling"; }

public:
    void setInputSPHObject(std::shared_ptr<SPHObject> sphObj);

    ///
    /// \brief How many times to resolve and correct position. This is useful when colliding
    /// with multiple objects or in a corner of another object
    ///
    void setNumberOfIterations(int iterations) { this->m_iterations = iterations; }

    void setDetection(std::shared_ptr<ImplicitGeometryToPointSetCD> colDetect) { this->m_colDetect = colDetect; }

    ///
    /// \brief Resolve SPH particle positions
    ///
    void handle(
        const CDElementVector<CollisionElement>& elementsA,
        const CDElementVector<CollisionElement>& elementsB) override;

protected:
    ///
    /// \brief Solves positiona and corrects velocity of individual particle
    ///
    void solve(Vec3d& pos, Vec3d& velocity, const Vec3d& penetrationVector);

private:
    std::shared_ptr<ImplicitGeometryToPointSetCD> m_colDetect = nullptr;
    int    m_iterations       = 1;
    double m_boundaryFriction = 0.0;
};
} // end namespace imstk
