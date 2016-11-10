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

#include "imstkPenaltyMeshToRigidCH.h"

#include "imstkCollidingObject.h"
#include "imstkCollisionData.h"
#include "imstkDeformableObject.h"

#include <g3log/g3log.hpp>

namespace imstk 
{

void
PenaltyMeshToRigidCH::computeContactForces()
{
    auto deformableObj = std::dynamic_pointer_cast<DeformableObject>(m_object);

    if (deformableObj == nullptr)
    {
        LOG(WARNING) << "PenaltyMeshToRigidCH::computeContactForces error: "
                     << m_object->getName() << " is not a deformable object. ";
        return;
    }

    // Get current force vector
    auto force = deformableObj->getContactForce();
    auto velVector = deformableObj->getVelocities();

    Vec3d nodalForce;
    Vec3d velocityProjection;
    int nodeDofID;
    double depth;
    // If collision data, append forces
    if (!m_colData.PDMeshAnaColData.empty())
    {
        for (const auto& cd : m_colData.PDMeshAnaColData)
        {
            nodeDofID = 3 * cd.nodeId;
            velocityProjection = Vec3d(velVector(nodeDofID), velVector(nodeDofID+1), velVector(nodeDofID+2));
            velocityProjection = cd.penetrationVector.dot(velocityProjection) * cd.penetrationVector;
            depth = cd.penetrationVector.norm();

            nodalForce = -m_stiffness * cd.penetrationVector - m_damping * velocityProjection;

            force(nodeDofID) = nodalForce.x();
            force(nodeDofID + 1) = nodalForce.y();
            force(nodeDofID + 2) = nodalForce.z();
        }
    }
}

}
