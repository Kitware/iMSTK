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

#include "imstkPickingCH.h"
#include "imstkDeformableObject.h"

namespace imstk
{
void
PickingCH::processCollisionData()
{
    if (m_object)
    {
        this->addPickConstraints(m_object);
    }
    else
    {
        LOG(WARNING) << "PickingCH::handleCollision error: "
                     << "no picking collision handling available the object";
    }
}

void
PickingCH::addPickConstraints(std::shared_ptr<DeformableObject> deformableObj)
{
    m_DynamicLinearProjConstraints->clear();

    if (m_colData.NodePickData.empty())
    {
        return;
    }

    if (deformableObj == nullptr)
    {
        LOG(WARNING) << "PenaltyRigidCH::addPickConstraints error: "
                     << " not a deformable object.";
        return;
    }

    const auto& Uprev = deformableObj->getDisplacements();
    const auto& Vprev = deformableObj->getVelocities();

    auto PhysTetMesh = std::dynamic_pointer_cast<PointSet>(deformableObj->getPhysicsGeometry());
    auto dT = std::dynamic_pointer_cast<FEMDeformableBodyModel>(m_object->getDynamicalModel())->getTimeIntegrator()->getTimestepSize();

    // If collision data, append LPC constraints
    for (const auto& CD : m_colData.NodePickData)
    {
        auto nodeDof = 3 * CD.nodeId;
        auto vprev = Vec3d(Vprev(nodeDof), Vprev(nodeDof + 1), Vprev(nodeDof + 2));
        auto uprev = Vec3d(Uprev(nodeDof), Uprev(nodeDof + 1), Uprev(nodeDof + 2));
        auto x = (CD.ptPos + PhysTetMesh->getVertexPosition(CD.nodeId) -
                  PhysTetMesh->getInitialVertexPosition(CD.nodeId) - uprev) / dT - vprev;

        auto pickProjector = LinearProjectionConstraint(CD.nodeId, true);
        pickProjector.setProjectorToDirichlet(CD.nodeId, x);

        m_DynamicLinearProjConstraints->push_back(pickProjector);
    }
}
}