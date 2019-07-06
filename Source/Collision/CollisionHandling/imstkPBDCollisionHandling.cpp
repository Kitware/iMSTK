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
#include <memory>
#include <g3log/g3log.hpp>

namespace imstk
{
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
    if (m_PBDSolver)
    {
        m_PBDSolver->addCollisionConstraints(&m_PBDConstraints);
    }
    else
    {
        LOG(WARNING) << "Error: PBDCollisionHandling: no PbdSolver found to handle the Collision constraints...";
    }
}

void
PBDCollisionHandling::generatePBDConstraints()
{
    // clear the constraints before populating wit new ones
    m_PBDConstraints.clear();

    const auto dynaModel1 = std::static_pointer_cast<PbdModel>(m_pbdObject1->getDynamicalModel());
    const auto dynaModel2 = std::static_pointer_cast<PbdModel>(m_pbdObject2->getDynamicalModel());

    const auto colGeo2 = std::static_pointer_cast<SurfaceMesh>(m_pbdObject2->getCollidingGeometry());

    const auto map1 = m_pbdObject1->getPhysicsToCollidingMap();
    const auto map2 = m_pbdObject2->getPhysicsToCollidingMap();

    //std::cout << "EE: " << m_colData->EEColData.size() << "TV: " << m_colData->TVColData.size() << std::endl;

    // Generate edge-edge pbd constraints
    for (auto& colData : m_colData->EEColData)
    {
        auto c = std::make_shared<PbdEdgeEdgeConstraint>();

        size_t edgeA1, edgeA2;
        if (map1)
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
        if (map2)
        {
            edgeB1 = map2->getMapIdx(colData.edgeIdB.first);
            edgeB2 = map2->getMapIdx(colData.edgeIdB.second);
        }
        else
        {
            edgeB1 = colData.edgeIdB.first;
            edgeB2 = colData.edgeIdB.second;
        }

        c->initConstraint(dynaModel1, edgeA1, edgeA2,
                          dynaModel2, edgeB1, edgeB2);

        m_PBDConstraints.push_back(c);
    }

    // Generate triangle-vertex pbd constraints
    for (auto& colData : m_colData->TVColData)
    {
        const auto& triVerts = colGeo2->getTrianglesVertices()[colData.triIdA];

        const auto c = std::make_shared<PbdPointTriangleConstraint>();

        size_t v1, v2, v3;
        if (map2)
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

        c->initConstraint(dynaModel1,
                          colData.vertexIdB,
                          dynaModel2,
                          v1,
                          v2,
                          v3);

        m_PBDConstraints.push_back(c);
    }
    //TODO: generating PbdPointTriangleConstraint from the VTColData should be added
}
}