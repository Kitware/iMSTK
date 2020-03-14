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

#include "imstkPBDCollisionHandling.h"
#include "imstkCollisionData.h"
#include "imstkDeformableObject.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdEdgeEdgeCollisionConstraint.h"
#include "imstkPbdPointTriCollisionConstraint.h"
#include "imstkPbdSolver.h"
#include "imstkParallelUtils.h"

#include <memory>
#include <g3log/g3log.hpp>

namespace imstk
{
PBDCollisionHandling::PBDCollisionHandling(const Side&                          side,
                                           const std::shared_ptr<CollisionData> colData,
                                           std::shared_ptr<CollidingObject>     obj1,
                                           std::shared_ptr<CollidingObject>     obj2) :
    CollisionHandling(Type::PBD, side, colData),
    m_pbdObject1(std::dynamic_pointer_cast<PbdObject>(obj1)),
    m_pbdObject2(std::dynamic_pointer_cast<PbdObject>(obj2))
{
}

PBDCollisionHandling::~PBDCollisionHandling()
{
    for (const auto ptr: m_EEConstraintPool)
    {
        delete ptr;
    }
    for (const auto ptr: m_VTConstraintPool)
    {
        delete ptr;
    }
}

void
PBDCollisionHandling::processCollisionData()
{
    /*if (auto deformableObj = std::dynamic_pointer_cast<DeformableObject>(m_object))
    {
        this->computeContactForcesDiscreteDeformable(deformableObj);
    }
    else if (auto analyticObj = std::dynamic_pointer_cast<CollidingObject>(m_object))
    {
        this->computeContactForcesAnalyticRigid(analyticObj);
    }
    else
    {
        LOG(WARNING) << "PenaltyRigidCH::computeContactForces error: "
                     << "no penalty collision handling available for " << m_object->getName()
                     << " (rigid mesh not yet supported).";
    }*/
    this->generatePBDConstraints();

    CHECK(m_PBDSolver) << "No PbdSolver found to handle the Collision constraints!";

    m_PBDSolver->addCollisionConstraints(&m_PBDConstraints);
}

void
PBDCollisionHandling::generatePBDConstraints()
{
    const auto dynaModel1 = std::static_pointer_cast<PbdModel>(m_pbdObject1->getDynamicalModel());
    const auto dynaModel2 = std::static_pointer_cast<PbdModel>(m_pbdObject2->getDynamicalModel());
    const auto colGeo2    = std::static_pointer_cast<SurfaceMesh>(m_pbdObject2->getCollidingGeometry());

    const auto map1 = m_pbdObject1->getPhysicsToCollidingMap();
    const auto map2 = m_pbdObject2->getPhysicsToCollidingMap();

//    std::cout << "EE: " << m_colData->EEColData.getSize() << "TV: " << m_colData->VTColData.getSize() << std::endl;

    // Generate edge-edge pbd constraints
    if (m_EEConstraintPool.size() < m_colData->EEColData.getSize())
    {
        for (size_t i = m_EEConstraintPool.size(); i < m_colData->EEColData.getSize(); ++i)
        {
            m_EEConstraintPool.push_back(new PbdEdgeEdgeConstraint);
        }
    }

    ParallelUtils::parallelFor(m_colData->EEColData.getSize(),
        [&] (const size_t idx)
        {
            const auto& colData = m_colData->EEColData[idx];

            size_t edgeA1, edgeA2;
            if (map1 && map1->getType() == GeometryMap::Type::OneToOne)
            {
                edgeA1 = map1->getMapIdx(colData.edgeIdA.first);
                edgeA2 = map1->getMapIdx(colData.edgeIdA.second);
            }
            else
            {
                edgeA1 = colData.edgeIdA.first;
                edgeA2 = colData.edgeIdA.second;
            }

            size_t edgeB1, edgeB2;
            if (map2 && map2->getType() == GeometryMap::Type::OneToOne)
            {
                edgeB1 = map2->getMapIdx(colData.edgeIdB.first);
                edgeB2 = map2->getMapIdx(colData.edgeIdB.second);
            }
            else
            {
                edgeB1 = colData.edgeIdB.first;
                edgeB2 = colData.edgeIdB.second;
            }

            const auto constraint = m_EEConstraintPool[idx];
            constraint->initConstraint(dynaModel1, edgeA1, edgeA2, dynaModel2, edgeB1, edgeB2);
    });

    // Generate vertex-triangle pbd constraints
    if (m_VTConstraintPool.size() < m_colData->VTColData.getSize())
    {
        for (size_t i = m_VTConstraintPool.size(); i < m_colData->VTColData.getSize(); ++i)
        {
            m_VTConstraintPool.push_back(new PbdPointTriangleConstraint);
        }
    }

    ParallelUtils::parallelFor(m_colData->VTColData.getSize(),
        [&] (const size_t idx)
        {
            const auto& colData  = m_colData->VTColData[idx];
            const auto& triVerts = colGeo2->getTrianglesVertices()[colData.triIdx];

            size_t v1, v2, v3;
            if (map2 && map2->getType() == GeometryMap::Type::OneToOne)
            {
                v1 = map2->getMapIdx(triVerts[0]);
                v2 = map2->getMapIdx(triVerts[1]);
                v3 = map2->getMapIdx(triVerts[2]);
            }
            else
            {
                v1 = triVerts[0];
                v2 = triVerts[1];
                v3 = triVerts[2];
            }

            const auto constraint = m_VTConstraintPool[idx];
            constraint->initConstraint(dynaModel1, colData.vertexIdx, dynaModel2, v1, v2, v3);
    });

    // Copy constraints
    m_PBDConstraints.resize(0);
    m_PBDConstraints.reserve(m_colData->EEColData.getSize() + m_colData->VTColData.getSize());

    for (size_t i = 0; i < m_colData->EEColData.getSize(); ++i)
    {
        m_PBDConstraints.push_back(static_cast<PbdCollisionConstraint*>(m_EEConstraintPool[i]));
    }

    for (size_t i = 0; i < m_colData->VTColData.getSize(); ++i)
    {
        m_PBDConstraints.push_back(static_cast<PbdCollisionConstraint*>(m_VTConstraintPool[i]));
    }
}
}
