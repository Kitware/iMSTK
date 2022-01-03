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
#include "imstkCollisionData.h"
#include "imstkFeDeformableObject.h"
#include "imstkFEMDeformableBodyModel.h"
#include "imstkParallelUtils.h"
#include "imstkRbdConstraint.h"
#include "imstkRigidObject2.h"

namespace imstk
{
void
PenaltyCH::setInputFeObject(std::shared_ptr<FeDeformableObject> feObj)
{
    setInputObjectA(feObj);
}

void
PenaltyCH::setInputRbdObject(std::shared_ptr<RigidObject2> rbdObj)
{
    setInputObjectB(rbdObj);
}

std::shared_ptr<FeDeformableObject>
PenaltyCH::getInputFeObject()
{
    return std::dynamic_pointer_cast<FeDeformableObject>(getInputObjectA());
}

std::shared_ptr<RigidObject2>
PenaltyCH::getInputRbdObject()
{
    return std::dynamic_pointer_cast<RigidObject2>(getInputObjectB());
}

void
PenaltyCH::handle(
    const std::vector<CollisionElement>& elementsA,
    const std::vector<CollisionElement>& elementsB)
{
    auto deformableObj = std::dynamic_pointer_cast<FeDeformableObject>(getInputObjectA());
    auto rbdObj = std::dynamic_pointer_cast<RigidObject2>(getInputObjectB());

    if (deformableObj != nullptr)
    {
        this->computeContactForcesDiscreteDeformable(elementsA, deformableObj);
    }
    if (rbdObj != nullptr)
    {
        this->computeContactForcesAnalyticRigid(elementsB, rbdObj);
    }
    else
    {
        LOG(FATAL) << "no penalty collision handling available for " << getInputObjectA()->getName()
                   << " (rigid mesh not yet supported).";
    }
}

void
PenaltyCH::computeContactForcesAnalyticRigid(
    const std::vector<CollisionElement>& elements,
    std::shared_ptr<RigidObject2>        analyticObj)
{
    if (elements.empty())
    {
        return;
    }

    // Sum forces (only supports PointDirection contacts)
    Vec3d force = Vec3d::Zero();
    for (size_t i = 0; i < elements.size(); i++)
    {
        const CollisionElement& elem = elements[i];
        if (elem.m_type == CollisionElementType::PointDirection)
        {
            const Vec3d  dir = elem.m_element.m_PointDirectionElement.dir;
            const double penetrationDepth = elem.m_element.m_PointDirectionElement.penetrationDepth;
            force += dir * ((penetrationDepth + 1.0) * (penetrationDepth + 1.0) - 1.0) * 10.0;
        }
    }

    // Apply as external force
    *analyticObj->getRigidBody()->m_force = force;
}

void
PenaltyCH::computeContactForcesDiscreteDeformable(
    const std::vector<CollisionElement>& elements,
    std::shared_ptr<FeDeformableObject>  deformableObj)
{
    if (elements.empty())
    {
        return;
    }

    // Get current force vector
    std::shared_ptr<FemDeformableBodyModel> model     = deformableObj->getFEMModel();
    Vectord&                                force     = model->getContactForce();
    const Vectord&                          velVector = model->getCurrentState()->getQDot();

    // If collision data, append forces
    ParallelUtils::SpinLock lock;
    ParallelUtils::parallelFor(elements.size(),
        [&](const size_t i)
        {
            const CollisionElement& elem = elements[i];
            if (elem.m_type == CollisionElementType::PointIndexDirection)
            {
                const Vec3d dir = elem.m_element.m_PointDirectionElement.dir;
                const double penetrationDepth = elem.m_element.m_PointDirectionElement.penetrationDepth;
                const int ptIndex = elem.m_element.m_PointIndexDirectionElement.ptIndex;

                const Vec3d penetrationVector = dir * penetrationDepth;
                const Eigen::Index nodeDofID  = static_cast<Eigen::Index>(3 * ptIndex);

                Vec3d velocityProjection = Vec3d(velVector(nodeDofID),
                    velVector(nodeDofID + 1),
                    velVector(nodeDofID + 2));
                velocityProjection = velocityProjection.dot(dir) * penetrationVector;

                const Vec3d nodalForce = -m_stiffness * penetrationVector - m_damping * velocityProjection;

                lock.lock();
                force(nodeDofID)     += nodalForce.x();
                force(nodeDofID + 1) += nodalForce.y();
                force(nodeDofID + 2) += nodalForce.z();
                lock.unlock();
            }
        });
}
} // end namespace imstk