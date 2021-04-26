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

#include "imstkAnalyticalGeometry.h"
#include "imstkCollisionData.h"
#include "imstkPbdPointNormalCollisionConstraint.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPBDPickingCH.h"
#include "imstkPbdSolver.h"
#include "imstkPointSet.h"
#include "imstkSurfaceMesh.h"

namespace imstk
{
PBDPickingCH::PBDPickingCH(const CollisionHandling::Side&       side,
                           const std::shared_ptr<CollisionData> colData,
                           std::shared_ptr<PbdObject>           pbdObj,
                           std::shared_ptr<CollidingObject>     pickObj) :
    CollisionHandling(Type::PBDPicking, side, colData),
    m_pbdObj(pbdObj),
    m_pickObj(pickObj),
    m_pbdCollisionSolver(std::make_shared<PbdCollisionSolver>())
{
    m_isPicking = false;
    m_pickedPtIdxOffset.clear();
}

PBDPickingCH::~PBDPickingCH()
{
    for (const auto ptr : m_ACConstraintPool)
    {
        delete ptr;
    }
}

void
PBDPickingCH::processCollisionData()
{
    CHECK(m_pbdObj != nullptr && m_pickObj != nullptr)
        << "Error: invalid input pbd objects for collision handling";

    if (m_isPicking)
    {
        this->updatePickConstraints();
    }
    else
    {
        this->generatePBDConstraints();
        if (m_PBDConstraints.size() == 0)
        {
            return;
        }

        m_pbdCollisionSolver->addCollisionConstraints(&m_PBDConstraints,
            m_pbdObj->getPbdModel()->getCurrentState()->getPositions(),
            m_pbdObj->getPbdModel()->getInvMasses(),
            nullptr,
            nullptr);
    }
}

void
PBDPickingCH::updatePickConstraints()
{
    if (m_pickedPtIdxOffset.size() == 0)
    {
        this->removePickConstraints();
        return;
    }

    std::shared_ptr<PbdModel>                model      = m_pbdObj->getPbdModel();
    std::shared_ptr<AnalyticalGeometry>      pickGeom   = std::dynamic_pointer_cast<AnalyticalGeometry>(m_pickObj->getCollidingGeometry());
    std::shared_ptr<VecDataArray<double, 3>> vertices   = model->getCurrentState()->getPositions();
    VecDataArray<double, 3>&                 vertexData = *vertices;
    for (auto iter = m_pickedPtIdxOffset.begin(); iter != m_pickedPtIdxOffset.end(); iter++)
    {
        auto rot = pickGeom->getRotation();
        vertexData[iter->first] = pickGeom->getPosition() + rot * iter->second;
    }
}

void
PBDPickingCH::addPickConstraints(std::shared_ptr<PbdObject> pbdObj, std::shared_ptr<CollidingObject> pickObj)
{
    if (m_colData->PColData.isEmpty())
    {
        return;
    }

    CHECK(pbdObj != nullptr && pickObj != nullptr)
        << "Error: no pdb object or colliding object.";

    std::shared_ptr<PbdModel>           model    = pbdObj->getPbdModel();
    std::shared_ptr<AnalyticalGeometry> pickGeom = std::dynamic_pointer_cast<AnalyticalGeometry>(pickObj->getCollidingGeometry());
    CHECK(pickGeom != nullptr) << "Colliding geometry is analytical geometry ";

    std::shared_ptr<VecDataArray<double, 3>> vertices   = model->getCurrentState()->getPositions();
    VecDataArray<double, 3>&                 vertexData = *vertices;

    ParallelUtils::SpinLock lock;
    ParallelUtils::parallelFor(m_colData->PColData.getSize(),
        [&](const size_t idx) {
            const auto& cd = m_colData->PColData[idx];
            if (m_pickedPtIdxOffset.find(cd.nodeIdx) == m_pickedPtIdxOffset.end() && model->getInvMass(cd.nodeIdx) != 0.0)
            {
                auto pv  = cd.penetrationVector;
                auto rot = pickGeom->getRotation().transpose();
                auto relativePos = rot * (vertexData[cd.nodeIdx] - pv - pickGeom->getPosition());

                lock.lock();
                m_pickedPtIdxOffset[cd.nodeIdx] = relativePos;
                model->setFixedPoint(cd.nodeIdx);
                vertexData[cd.nodeIdx] = pickGeom->getPosition() + rot.transpose() * relativePos;
                lock.unlock();
            }
    });
}

void
PBDPickingCH::removePickConstraints()
{
    std::shared_ptr<PbdModel> model = m_pbdObj->getPbdModel();
    m_isPicking = false;
    for (auto iter = m_pickedPtIdxOffset.begin(); iter != m_pickedPtIdxOffset.end(); ++iter)
    {
        model->setPointUnfixed(iter->first);
    }
    m_pickedPtIdxOffset.clear();
}

void
PBDPickingCH::activatePickConstraints()
{
    if (!m_colData->PColData.isEmpty())
    {
        this->addPickConstraints(m_pbdObj, m_pickObj);
        m_isPicking = true;
    }
}

void
PBDPickingCH::generatePBDConstraints()
{
    std::shared_ptr<PbdCollisionConstraintConfig> config = m_pbdObj->getPbdModel()->getParameters()->collisionParams;

    {
        // \todo: Pool not implemented correctly (see PbdCollisionHandling for proper trick), for now, just delete
        for (size_t i = 0; i < m_ACConstraintPool.size(); ++i)
        {
            delete m_ACConstraintPool[i];
        }
    }
    m_ACConstraintPool.resize(0);
    m_ACConstraintPool.reserve(m_colData->PColData.getSize());
    for (size_t i = 0; i < m_colData->PColData.getSize(); ++i)
    {
        m_ACConstraintPool.push_back(new PbdPointNormalCollisionConstraint);
    }

    std::shared_ptr<PointSet>      pointSet = std::dynamic_pointer_cast<PointSet>(m_pbdObj->getPhysicsGeometry());
    const VecDataArray<double, 3>& vertices = *pointSet->getVertexPositions();
    ParallelUtils::parallelFor(m_colData->PColData.getSize(),
        [&](const size_t idx)
        {
            const Vec3d& initPos = vertices[m_colData->PColData[idx].nodeIdx];
            const Vec3d& penetrationVector = -m_colData->PColData[idx].penetrationVector; // Vector to resolve
            m_ACConstraintPool[idx]->initConstraint(config, initPos + penetrationVector, penetrationVector, m_colData->PColData[idx].nodeIdx);
        });

    // Copy constraints
    m_PBDConstraints.resize(0);
    m_PBDConstraints.reserve(m_colData->PColData.getSize());
    for (size_t i = 0; i < m_colData->PColData.getSize(); ++i)
    {
        m_PBDConstraints.push_back(static_cast<PbdCollisionConstraint*>(m_ACConstraintPool[i]));
    }
}
}
