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

namespace imstk
{
void SPHCollisionHandling::setBoundaryFriction(const Real friction)
{
    m_BoundaryFriction = friction;

    if(m_BoundaryFriction < 0.0 || m_BoundaryFriction > 1.0)
    {
        LOG(WARNING) << "Invalid frictionLength coefficient (it must be in [0, 1])";
        if(m_BoundaryFriction < 0)
        {
            m_BoundaryFriction = 0;
        }
        else if(m_BoundaryFriction > static_cast<Real>(1.0))
        {
            m_BoundaryFriction = static_cast<Real>(1.0);
        }
    }
}

void SPHCollisionHandling::processCollisionData()
{
    const auto& SPHModel = m_SPHObject->getSPHModel();
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, !SPHModel) << "SPH model was not initialized";
#endif

    auto& state = SPHModel->getState();
    for(const auto& cd : m_colData->MAColData)
    {
        const auto pidx = cd.nodeId; // Fluid particle index
        auto n = cd.penetrationVector;  // This vector should point into solid object

        // Correct particle position:
        state.getPositions()[pidx] -= n;

        // Correct particle velocity: slip boundary condition with friction
        auto& vel = state.getVelocities()[pidx];
        const auto nLengthSqr = n.squaredNorm();
        if(nLengthSqr > Real(1e-20))     // Normalize n
        {
            n /= std::sqrt(nLengthSqr);
        }
        const auto vn = vel.dot(n);
        vel -= n * vn;     // From now, vel is parallel with the solid surface

        if(m_BoundaryFriction > Real(1e-20))
        {
            const auto velLength  = vel.norm();
            const auto frictionLength = -vn * m_BoundaryFriction;
            if(frictionLength < velLength && velLength > Real(1e-10))
            {
                vel -= (vel / velLength) * frictionLength;     // Subtract a friction from velocity, which is proportional to the amount of penetration
            }
            else
            {
                vel = Vec3r::Zero();
            }
        }
    }
}
} // end namespace imstk
