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

#include "imstkPBDPickingCH.h"
#include "imstkAnalyticalGeometry.h"
#include "imstkCollisionData.h"
#include "imstkParallelUtils.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdPointPointConstraint.h"
#include "imstkPbdSolver.h"
#include "imstkPointSet.h"
#include "imstkSurfaceMesh.h"

namespace imstk
{
PBDPickingCH::PBDPickingCH() :
    m_isPicking(false),
    m_isPrevPicking(false)
{
    m_pickedPtIdxOffset.clear();
}

void
PBDPickingCH::handle(
    const std::vector<CollisionElement>& elementsA,
    const std::vector<CollisionElement>& imstkNotUsed(elementsB))
{
    std::shared_ptr<PbdObject>       pbdObj  = std::dynamic_pointer_cast<PbdObject>(getInputObjectA());
    std::shared_ptr<CollidingObject> pickObj = getInputObjectB();

    CHECK(pbdObj != nullptr && pickObj != nullptr)
        << "PBDPickingCH::handleCollision error: "
        << "no picking collision handling available the object";

    // If started picking
    if (!m_isPrevPicking && m_isPicking)
    {
        this->addPickConstraints(elementsA, pbdObj, pickObj);
    }
    // If stopped picking
    if (!m_isPicking && m_isPrevPicking)
    {
        // Unfix the points
        std::shared_ptr<PbdModel> model = pbdObj->getPbdModel();
        for (auto iter = m_pickedPtIdxOffset.begin(); iter != m_pickedPtIdxOffset.end(); ++iter)
        {
            model->setPointUnfixed(iter->first);
        }
        m_pickedPtIdxOffset.clear();
    }

    if (m_isPicking)
    {
        if (m_pickedPtIdxOffset.size() == 0)
        {
            this->endPick();
            return;
        }

        std::shared_ptr<PbdModel>           model    = pbdObj->getPbdModel();
        std::shared_ptr<AnalyticalGeometry> pickGeom =
            std::dynamic_pointer_cast<AnalyticalGeometry>(pickObj->getCollidingGeometry());
        std::shared_ptr<VecDataArray<double, 3>> vertices   = model->getCurrentState()->getPositions();
        VecDataArray<double, 3>&                 vertexData = *vertices;
        for (auto iter = m_pickedPtIdxOffset.begin(); iter != m_pickedPtIdxOffset.end(); iter++)
        {
            auto rot = pickGeom->getRotation();
            vertexData[iter->first] = pickGeom->getPosition() + rot * iter->second;
        }
    }
    else
    {
        this->generatePBDConstraints(elementsA);

        // Immediately do the constraint solve
        // Does not solve for restitution or friction
        for (const auto& constraint : m_constraints)
        {
            constraint->solvePosition();
        }
    }

    // Push back the picking state
    m_isPrevPicking = m_isPicking;
}

void
PBDPickingCH::addPickConstraints(const std::vector<CollisionElement>& elements,
                                 std::shared_ptr<PbdObject> pbdObj, std::shared_ptr<CollidingObject> pickObj)
{
    CHECK(pbdObj != nullptr && pickObj != nullptr)
        << "PBDPickingCH:addPickConstraints error: "
        << "no pdb object or colliding object.";

    std::shared_ptr<PbdModel>           model    = pbdObj->getPbdModel();
    std::shared_ptr<AnalyticalGeometry> pickGeom = std::dynamic_pointer_cast<AnalyticalGeometry>(pickObj->getCollidingGeometry());
    CHECK(pickGeom != nullptr) << "Colliding geometry is analytical geometry ";

    std::shared_ptr<VecDataArray<double, 3>> verticesPtr = model->getCurrentState()->getPositions();
    VecDataArray<double, 3>&                 vertices    = *verticesPtr;

    // For every collision element record the offsets and fix the points in the PbdModel
    ParallelUtils::SpinLock lock;
    ParallelUtils::parallelFor(elements.size(),
        [&](const size_t idx)
        {
            const CollisionElement& elem = elements[idx];
            if (elem.m_type == CollisionElementType::PointIndexDirection)
            {
                const PointIndexDirectionElement& pdElem = elem.m_element.m_PointIndexDirectionElement;
                if (m_pickedPtIdxOffset.find(pdElem.ptIndex) == m_pickedPtIdxOffset.end()
                    && model->getInvMass(pdElem.ptIndex) != 0.0)
                {
                    const Vec3d pv  = pdElem.dir * pdElem.penetrationDepth;
                    const Mat3d rot = pickGeom->getRotation().transpose();
                    const Vec3d relativePos = rot * (vertices[pdElem.ptIndex] - pv - pickGeom->getPosition());

                    lock.lock();
                    m_pickedPtIdxOffset[pdElem.ptIndex] = relativePos;
                    model->setFixedPoint(pdElem.ptIndex);
                    vertices[pdElem.ptIndex] = pickGeom->getPosition() + rot.transpose() * relativePos;
                    lock.unlock();
                }
            }
        }, elements.size() > 100);
}

void
PBDPickingCH::generatePBDConstraints(const std::vector<CollisionElement>& elements)
{
    // Only constraint when picking is on
    if (m_isPicking)
    {
        std::shared_ptr<PbdObject>       pbdObj  = std::dynamic_pointer_cast<PbdObject>(getInputObjectA());
        std::shared_ptr<CollidingObject> pickObj = getInputObjectB();

        CHECK(pbdObj != nullptr && pickObj != nullptr)
            << "PBDPickingCH:addPickConstraints error: "
            << "no pdb object or colliding object.";

        m_constraints.resize(0);
        m_constraints.reserve(elements.size());

        constraintPts.clear();
        constraintVels.clear();

        // Get the geometry and some attributes that we can assume are there so long as this is a pbdObj
        auto pointSet = std::dynamic_pointer_cast<PointSet>(pbdObj->getPhysicsGeometry());

        std::shared_ptr<VecDataArray<double, 3>> verticesPtr   = pointSet->getVertexPositions();
        auto                                     velocitiesPtr = std::dynamic_pointer_cast<VecDataArray<double, 3>>(pointSet->getVertexAttribute("Velocities"));
        auto                                     invMassesPtr  = std::dynamic_pointer_cast<DataArray<double>>(pointSet->getVertexAttribute("InvMass"));

        VecDataArray<double, 3>& vertices   = *verticesPtr;
        VecDataArray<double, 3>& velocities = *velocitiesPtr;
        const DataArray<double>& invMasses  = *invMassesPtr;

        for (size_t i = 0; i < elements.size(); i++)
        {
            const CollisionElement& elem = elements[i];
            if (elem.m_type == CollisionElementType::PointIndexDirection)
            {
                const PointIndexDirectionElement& pdElem  = elem.m_element.m_PointIndexDirectionElement;
                const Vec3d&                      initPos = vertices[pdElem.ptIndex];
                const Vec3d                       penetrationVector = -pdElem.dir * pdElem.penetrationDepth; // Vector to resolve

                constraintPts.push_back(initPos - penetrationVector);
                constraintVels.push_back(Vec3d::Zero());

                // Mapped indices not supported
                m_constraints.push_back(std::make_shared<PbdPointPointConstraint>());
                m_constraints[i]->initConstraint(
                    { &vertices[pdElem.ptIndex], invMasses[pdElem.ptIndex], &velocities[pdElem.ptIndex] },
                    { &constraintPts.back(), 0.0, &constraintVels.back() },
                    1.0, 0.0);
            }
        }
    }
}
}