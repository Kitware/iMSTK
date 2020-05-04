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

#include "imstkPenaltyCH.h"

#include "imstkCollidingObject.h"
#include "imstkCollisionData.h"
#include "imstkDeformableObject.h"
#include "imstkParallelUtils.h"

#include <g3log/g3log.hpp>

namespace imstk
{
PenaltyCH::PenaltyCH(const Side&                             side,
                     const std::shared_ptr<CollisionData>&   colData,
                     const std::shared_ptr<CollidingObject>& obj) :
    CollisionHandling(Type::Penalty, side, colData), m_object(obj)
{
    LOG_IF(FATAL, (!obj)) << "Empty colliding object";
}

void
PenaltyCH::processCollisionData()
{
    if (auto deformableObj = std::dynamic_pointer_cast<FeDeformableObject>(m_object))
    {
        this->computeContactForcesDiscreteDeformable(deformableObj);
    }
    //else if(auto obj = std::dynamic_pointer_cast<RigidObject>(m_object)) computeContactForcesDiscreteRigid
    else if (auto analyticObj = std::dynamic_pointer_cast<CollidingObject>(m_object))
    {
        this->computeContactForcesAnalyticRigid(analyticObj);
    }
    else
    {
        LOG(FATAL) << "no penalty collision handling available for " << m_object->getName()
                   << " (rigid mesh not yet supported).";
    }
}

void
PenaltyCH::computeContactForcesAnalyticRigid(const std::shared_ptr<CollidingObject>& analyticObj)
{
    if (m_colData->PDColData.isEmpty())
    {
        return;
    }

    CHECK(analyticObj != nullptr) << m_object->getName() << " is not a colliding object";

    // If collision data is valid, append forces
    Vec3d force(0., 0., 0.);
    for (size_t i = 0; i < m_colData->PDColData.getSize(); ++i)
    {
        const auto& cd = m_colData->PDColData[i];
        if (m_side == CollisionHandling::Side::A)
        {
            force -= cd.dirAtoB * ((cd.penetrationDepth + 1) * (cd.penetrationDepth + 1) - 1) * 10;
        }
        else if (m_side == CollisionHandling::Side::B)
        {
            force += cd.dirAtoB * ((cd.penetrationDepth + 1) * (cd.penetrationDepth + 1) - 1) * 10;
        }
    }

    // Update object contact force
    analyticObj->appendForce(force);
}

void
PenaltyCH::computeContactForcesDiscreteDeformable(const std::shared_ptr<FeDeformableObject>& deformableObj)
{
    if (m_colData->MAColData.isEmpty())
    {
        return;
    }

    CHECK(deformableObj != nullptr) << "PenaltyRigidCH::computeContactForcesDiscreteDeformable error: "
                                    << m_object->getName() << " is not a deformable object.";

    // Get current force vector
    auto&       force     = deformableObj->getContactForce();
    const auto& velVector = deformableObj->getVelocities();

    // If collision data, append forces
    ParallelUtils::SpinLock lock;
    ParallelUtils::parallelFor(m_colData->MAColData.getSize(),
        [&](const size_t idx) {
            const auto& cd       = m_colData->MAColData[idx];
            const auto nodeDofID = static_cast<Eigen::Index>(3 * cd.nodeIdx);
            const auto unit      = cd.penetrationVector / cd.penetrationVector.norm();

            auto velocityProjection = Vec3d(velVector(nodeDofID),
                                            velVector(nodeDofID + 1),
                                            velVector(nodeDofID + 2));
            velocityProjection = velocityProjection.dot(unit) * cd.penetrationVector;

            const auto nodalForce = -m_stiffness * cd.penetrationVector - m_damping * velocityProjection;

            lock.lock();
            force(nodeDofID)     += nodalForce.x();
            force(nodeDofID + 1) += nodalForce.y();
            force(nodeDofID + 2) += nodalForce.z();
            lock.unlock();
    });
}
} // end namespace imstk
