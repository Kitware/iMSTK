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

#include "imstkBoneDrillingCH.h"

#include "imstkCollidingObject.h"
#include "imstkTetrahedralMesh.h"
#include "imstkCollisionData.h"
#include "imstkDeviceTracker.h"
#include "imstkMath.h"

#include <g3log/g3log.hpp>

namespace imstk
{
BoneDrillingCH::BoneDrillingCH(const Side& side,
                               const CollisionData& colData,
                               std::shared_ptr<CollidingObject> bone,
                               std::shared_ptr<CollidingObject> drill) :
    CollisionHandling(Type::BoneDrilling, side, colData),
    m_drill(drill),
    m_bone(bone)
{
    auto boneMesh = std::dynamic_pointer_cast<TetrahedralMesh>(m_bone->getCollidingGeometry());

    if (!boneMesh)
    {
        LOG(WARNING) << "BoneDrillingCH::BoneDrillingCH Error:The bone colliding geometry is not a mesh!";
    }

    // Initialize bone density values
    for (int i = 0; i < boneMesh->getNumVertices(); ++i)
    {
        m_nodalDensity.push_back(m_initialBoneDensity);
    }

    for (int i = 0; i < boneMesh->getNumVertices(); ++i)
    {
        m_nodeRemovalStatus.push_back(false);
    }

    m_nodalCardinalSet.resize(boneMesh->getNumVertices());
    for (int i = 0; i < boneMesh->getNumVertices(); ++i)
    {
        std::vector<int> row;
        m_nodalCardinalSet.push_back(row);
    }

    // Pre-compute the nodal cardinality set
    for (int i = 0; i < boneMesh->getNumTetrahedra(); ++i)
    {
        for (auto& vert : boneMesh->getTetrahedronVertices(i))
        {
            m_nodalCardinalSet[vert].push_back(i);
        }
    }
}

void
BoneDrillingCH::erodeBone()
{
    auto boneTetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(m_bone->getCollidingGeometry());

    for (auto& cd : m_colData.MAColData)
    {
        if (m_nodeRemovalStatus[cd.nodeId])
        {
            continue;
        }

        m_nodalDensity[cd.nodeId] -= 0.001*(m_angularSpeed / m_BoneHardness)*m_stiffness*cd.penetrationVector.norm()*0.001;

        if (m_nodalDensity[cd.nodeId] <= 0.)
        {
            m_erodedNodes.push_back(cd.nodeId);
            m_nodeRemovalStatus[cd.nodeId] = true;

            // tag the tetra that will be removed
            for (auto& tetId : m_nodalCardinalSet[cd.nodeId])
            {
                boneTetMesh->setTetrahedraAsRemoved(tetId);
                boneTetMesh->setTopologyChangedFlag(true);
            }
        }
    }
}

void
BoneDrillingCH::processCollisionData()
{
    // Check if any collisions
    const auto devicePosition = m_drill->getCollidingGeometry()->getTranslation();
    if (m_colData.MAColData.empty())
    {
        // Set the visual object position same as the colliding object position
        m_drill->getVisualGeometry()->setTranslation(devicePosition);
        return;
    }

    // Update visual object position

    // Aggregate collision data
    Vec3d t = Vec3d::Zero();
    double maxDepth = MIN_D;
    for (const auto& cd : m_colData.MAColData)
    {
        if (m_nodeRemovalStatus[cd.nodeId])
        {
            continue;
        }

        if (cd.penetrationVector.norm() > maxDepth)
        {
            maxDepth = cd.penetrationVector.norm();
            t = cd.penetrationVector;
        }
    }
    m_drill->getVisualGeometry()->setTranslation(devicePosition + t);

    // Spring force
    Vec3d force = m_stiffness * (m_drill->getVisualGeometry()->getTranslation() - devicePosition);

    // Damping force
    const double dt = 0.1; // Time step size to calculate the object velocity
    force += m_initialStep ? Vec3d(0.0, 0.0, 0.0) : m_damping * (devicePosition - m_prevPos) / dt;

    // Update object contact force
    m_drill->appendForce(force);

    // Decrease the density at the nodal points and remove if the density goes below 0
    this->erodeBone();

    // Housekeeping
    m_initialStep = false;
    m_prevPos = devicePosition;
}
}// iMSTK