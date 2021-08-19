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
void
SPHCollisionHandling::setInputSPHObject(std::shared_ptr<SPHObject> sphObj)
{
    setInputObjectA(sphObj);
}

void
SPHCollisionHandling::handle(
    const CDElementVector<CollisionElement>& elementsA,
    const CDElementVector<CollisionElement>& elementsB)
{
    std::shared_ptr<SPHObject> obj      = std::dynamic_pointer_cast<SPHObject>(getInputObjectA());
    std::shared_ptr<SPHModel>  sphModel = obj->getSPHModel();
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (!sphModel)) << "SPH model was not initialized";
#endif

    m_boundaryFriction = sphModel->getParameters()->m_frictionBoundary;
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (m_boundaryFriction<0.0 || m_boundaryFriction>1.0))
        << "Invalid boundary friction coefficient (value must be in [0, 1])";
#endif

    std::shared_ptr<SPHState>                state = sphModel->getCurrentState();
    std::shared_ptr<VecDataArray<double, 3>> positionsPtr  = state->getPositions();
    std::shared_ptr<VecDataArray<double, 3>> velocitiesPtr = state->getVelocities();
    VecDataArray<double, 3>&                 positions     = *positionsPtr;
    VecDataArray<double, 3>&                 velocities    = *velocitiesPtr;

    // Solve analytical collision
    for (int i = 0; i < m_iterations; i++)
    {
        // Coming into this CH, CD has already been computed
        if (i != 0 && m_colDetect != nullptr)
        {
            // Update the collision geometry
            obj->updateGeometries();
            // Compute collision again
            m_colDetect->update();
        }

        ParallelUtils::parallelFor(elementsA.getSize(), [&](const size_t j)
            {
                const CollisionElement& colElem = elementsA[j];
                if (colElem.m_type == CollisionElementType::PointIndexDirection)
                {
                    const int particleIndex = colElem.m_element.m_PointIndexDirectionElement.ptIndex;
                    const Vec3d& n     = -colElem.m_element.m_PointIndexDirectionElement.dir;
                    const double depth = colElem.m_element.m_PointIndexDirectionElement.penetrationDepth;
                    solve(positions[particleIndex], velocities[particleIndex], n * depth);
                }
            });
    }
}

void
SPHCollisionHandling::solve(Vec3d& pos, Vec3d& velocity, const Vec3d& penetrationVector)
{
    // Correct particle position
    pos -= penetrationVector;

    const auto nLengthSqr = penetrationVector.squaredNorm();
    if (nLengthSqr < 1.0e-20) // Normalize n
    {
        return;               // Too little penetration: ignore
    }
    const Vec3d n = penetrationVector / std::sqrt(nLengthSqr);

    // Correct particle velocity: slip boundary condition with friction
    const Vec3d  oldVel = velocity;
    const double vn     = oldVel.dot(n);

    // If particle is escaping the boundary, ignore it
    if (vn > 0)
    {
        Vec3d correctedVel = oldVel - vn * n; // From now, vel is parallel with the solid surface

        if (m_boundaryFriction > 1.0e-20)
        {
            const double velLength      = correctedVel.norm();
            const double frictionLength = vn * m_boundaryFriction; // This is always positive
            if (frictionLength < velLength && velLength > 1.0e-10)
            {
                correctedVel -= (correctedVel / velLength) * frictionLength; // Subtract a friction from velocity, which is proportional to the amount of penetration
            }
            else
            {
                correctedVel = Vec3d::Zero();
            }
        }

        velocity = correctedVel;
    }
}
} // end namespace imstk
