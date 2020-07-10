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

#include "imstkSPHCollisionHandling.h"
#include "imstkCollisionData.h"
#include "imstkImplicitGeometryToPointSetCD.h"
#include "imstkSPHModel.h"
#include "imstkSPHObject.h"

namespace imstk
{
SPHCollisionHandling::SPHCollisionHandling(const CollisionHandling::Side& side,
                                           std::shared_ptr<CollisionData> colData,
                                           std::shared_ptr<SPHObject>     obj) :
    CollisionHandling(Type::SPH, side, colData), m_SPHObject(obj)
{
    LOG_IF(FATAL, (!m_SPHObject)) << "Invalid SPH object";
}

void
SPHCollisionHandling::processCollisionData()
{
    const auto& SPHModel = m_SPHObject->getSPHModel();
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (!SPHModel)) << "SPH model was not initialized";
#endif

    const auto boundaryFriction = m_SPHObject->getSPHModel()->getParameters()->m_frictionBoundary;
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (boundaryFriction<0.0 || boundaryFriction>1.0))
        << "Invalid boundary friction coefficient (value must be in [0, 1])";
#endif

    SPHSimulationState&                   state = SPHModel->getState();
    std::function<void(uint32_t, Vec3d&)> solve =
        [&](const uint32_t pidx, const Vec3d& penetrationVector)
        {
            // Correct particle position
            state.getPositions()[pidx] -= penetrationVector;

            const auto nLengthSqr = penetrationVector.squaredNorm();
            if (nLengthSqr < Real(1e-20)) // Normalize n
            {
                return;                   // Too little penetration: ignore
            }
            const Vec3d n = penetrationVector / std::sqrt(nLengthSqr);

            // Correct particle velocity: slip boundary condition with friction
            const Vec3d& oldVel = state.getVelocities()[pidx];
            const double vn     = oldVel.dot(n);

            // If particle is escaping the boundary, ignore it
            if (vn > 0)
            {
                Vec3r correctedVel = oldVel - vn * n; // From now, vel is parallel with the solid surface

                if (boundaryFriction > Real(1e-20))
                {
                    const auto velLength      = correctedVel.norm();
                    const auto frictionLength = vn * boundaryFriction; // This is always positive
                    if (frictionLength < velLength && velLength > Real(1e-10))
                    {
                        correctedVel -= (correctedVel / velLength) * frictionLength; // Subtract a friction from velocity, which is proportional to the amount of penetration
                    }
                    else
                    {
                        correctedVel = Vec3r::Zero();
                    }
                }

                state.getVelocities()[pidx] = correctedVel;
            }
        };

    // Solve analytical collision
    for (int iter = 0; iter < m_iterations; iter++)
    {
        // Coming into this CH, CD has already been computed
        if (iter != 0)
        {
            // Update the collision geometry
            m_SPHObject->updateGeometries();
            // Compute collision again
            m_colDetect->computeCollisionData();
        }

        ParallelUtils::parallelFor(m_colData->MAColData.getSize(),
            [&](const size_t idx)
            {
                // Because of const, make some extra copies
                const MeshToAnalyticalCollisionDataElement& cd = m_colData->MAColData[idx];
                const uint32_t pidx     = cd.nodeIdx;           // Fluid particle index
                Vec3d penetrationVector = cd.penetrationVector; // This vector should point into solid object

                solve(pidx, penetrationVector);
            });
        // Solve point/direction based collision
        ParallelUtils::parallelFor(m_colData->PDColData.getSize(),
            [&](const size_t idx)
            {
                // Because of const, make some extra copies
                const PositionDirectionCollisionDataElement& cd = m_colData->PDColData[idx];
                const uint32_t pidx     = cd.nodeIdx;                       // Fluid particle index
                Vec3d penetrationVector = cd.dirAtoB * cd.penetrationDepth; // This vector should point into solid object

                solve(pidx, penetrationVector);
            });
    }
}
} // end namespace imstk
