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

#include <g3log/g3log.hpp>

namespace imstk
{
void
PenaltyCH::computeContactForces()
{
    if (auto deformableObj = std::dynamic_pointer_cast<DeformableObject>(m_object))
    {
        this->computeContactForcesDiscreteDeformable(deformableObj);
    }
    //else if (auto obj = std::dynamic_pointer_cast<RigidObject>(m_object)) computeContactForcesDiscreteRigid
    else if (auto analyticObj = std::dynamic_pointer_cast<CollidingObject>(m_object))
    {
        this->computeContactForcesAnalyticRigid(analyticObj);
    }
    else
    {
        LOG(WARNING) << "PenaltyRigidCH::computeContactForces error: "
                     << "no penalty collision handling available for " << m_object->getName()
                     << " (rigid mesh not yet supported).";
    }
}

void
PenaltyCH::computeContactForcesAnalyticRigid(std::shared_ptr<CollidingObject> analyticObj)
{
    if (m_colData.PDColData.empty())
    {
        return;
    }

    if(analyticObj == nullptr)
    {
        LOG(WARNING) << "PenaltyRigidCH::computeContactForcesAnalyticRigid error: "
                     << m_object->getName() << " is not a colliding object";

        return;
    }

    // If collision data is valid, append forces
    Vec3d force(0., 0., 0.);
    for(const auto& cd : m_colData.PDColData)
    {
        if (m_side == CollisionHandling::Side::A)
        {
            force -= cd.dirAtoB * ((cd.penetrationDepth+1)*(cd.penetrationDepth+1)-1)*10;
        }
        else if (m_side == CollisionHandling::Side::B)
        {
            force += cd.dirAtoB * ((cd.penetrationDepth+1)*(cd.penetrationDepth+1)-1)*10;
        }
    }

    // Update object contact force
    analyticObj->appendForce(force);
}

void
PenaltyCH::computeContactForcesDiscreteDeformable(std::shared_ptr<DeformableObject> deformableObj)
{
    if (m_colData.MAColData.empty())
    {
        return;
    }

    if (deformableObj == nullptr)
    {
        LOG(WARNING) << "PenaltyRigidCH::computeContactForcesDiscreteDeformable error: "
                     << m_object->getName() << " is not a deformable object.";
        return;
    }

    // Get current force vector
    auto& force = deformableObj->getContactForce();
    const auto& velVector = deformableObj->getVelocities();

    Vec3d nodalForce;
    Vec3d velocityProjection;
    size_t nodeDofID;

    // If collision data, append forces
    for (const auto& cd : m_colData.MAColData)
    {
        nodeDofID = 3 * cd.nodeId;
        velocityProjection = Vec3d(velVector(nodeDofID), velVector(nodeDofID + 1), velVector(nodeDofID + 2));

        auto unit = cd.penetrationVector / cd.penetrationVector.norm();
        velocityProjection = cd.penetrationVector.dot(unit) * cd.penetrationVector;

        nodalForce = -m_stiffness * cd.penetrationVector - m_damping * velocityProjection;

        force(nodeDofID) += nodalForce.x();
        force(nodeDofID + 1) += nodalForce.y();
        force(nodeDofID + 2) += nodalForce.z();
    }
}
}
