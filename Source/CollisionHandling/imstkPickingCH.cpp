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
#include "imstkFeDeformableObject.h"
#include "imstkParallelUtils.h"
#include "imstkPointSet.h"
#include "imstkFEMDeformableBodyModel.h"
#include "imstkTimeIntegrator.h"
#include "imstkCollisionData.h"
#include "imstkLinearProjectionConstraint.h"

namespace imstk
{
PickingCH::PickingCH(const CollisionHandling::Side&       side,
                     const std::shared_ptr<CollisionData> colData,
                     std::shared_ptr<FeDeformableObject>  obj) :
    CollisionHandling(Type::NodalPicking, side, colData),
    m_object(obj)
{
}

void
PickingCH::processCollisionData()
{
    CHECK(m_object != nullptr) << "PickingCH::handleCollision error: "
                               << "no picking collision handling available the object";

    this->addPickConstraints(m_object);
}

void
PickingCH::addPickConstraints(std::shared_ptr<FeDeformableObject> deformableObj)
{
    m_DynamicLinearProjConstraints->clear();

    if (m_colData->NodePickData.isEmpty())
    {
        return;
    }

    CHECK(deformableObj != nullptr) << "PenaltyRigidCH::addPickConstraints error: "
                                    << " not a deformable object.";

    std::shared_ptr<FEMDeformableBodyModel> model = deformableObj->getFEMModel();
    const Vectord&                          Uprev = model->getCurrentState()->getQ();
    const Vectord&                          Vprev = model->getCurrentState()->getQDot();

    auto PhysTetMesh = std::dynamic_pointer_cast<PointSet>(deformableObj->getPhysicsGeometry());
    auto dT = std::dynamic_pointer_cast<FEMDeformableBodyModel>(m_object->getDynamicalModel())->getTimeIntegrator()->getTimestepSize();

    // If collision data, append LPC constraints
    ParallelUtils::SpinLock lock;
    ParallelUtils::parallelFor(m_colData->NodePickData.getSize(),
        [&](const size_t idx) {
            const auto& cd     = m_colData->NodePickData[idx];
            const auto nodeDof = static_cast<Eigen::Index>(3 * cd.nodeIdx);
            const auto vprev   = Vec3d(Vprev(nodeDof), Vprev(nodeDof + 1), Vprev(nodeDof + 2));
            const auto uprev   = Vec3d(Uprev(nodeDof), Uprev(nodeDof + 1), Uprev(nodeDof + 2));
            const auto x       = (cd.ptPos + PhysTetMesh->getVertexPosition(cd.nodeIdx) -
                                  PhysTetMesh->getInitialVertexPosition(cd.nodeIdx) - uprev) / dT - vprev;

            auto pickProjector = LinearProjectionConstraint(cd.nodeIdx, true);
            pickProjector.setProjectorToDirichlet(static_cast<unsigned int>(cd.nodeIdx), x);

            lock.lock();
            m_DynamicLinearProjConstraints->push_back(std::move(pickProjector));
            lock.unlock();
    });
}
}
