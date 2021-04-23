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
#include "imstkCollisionData.h"
#include "imstkTetrahedralMesh.h"

namespace imstk
{
BoneDrillingCH::BoneDrillingCH(const Side&                          side,
                               const std::shared_ptr<CollisionData> colData,
                               std::shared_ptr<CollidingObject>     bone,
                               std::shared_ptr<CollidingObject>     drill) :
    CollisionHandling(Type::BoneDrilling, side, colData),
    m_bone(bone),
    m_drill(drill)
{
    auto boneMesh = std::dynamic_pointer_cast<TetrahedralMesh>(m_bone->getCollidingGeometry());

    CHECK(boneMesh != nullptr) << "Error:The bone colliding geometry is not a mesh!";

    // Initialize bone density values
    m_nodalDensity.reserve(boneMesh->getNumVertices());
    for (size_t i = 0; i < boneMesh->getNumVertices(); ++i)
    {
        m_nodalDensity.push_back(m_initialBoneDensity);
    }

    m_nodeRemovalStatus.reserve(boneMesh->getNumVertices());
    for (size_t i = 0; i < boneMesh->getNumVertices(); ++i)
    {
        m_nodeRemovalStatus.push_back(false);
    }

    m_nodalCardinalSet.reserve(boneMesh->getNumVertices());
    for (size_t i = 0; i < boneMesh->getNumVertices(); ++i)
    {
        std::vector<size_t> row;
        m_nodalCardinalSet.push_back(row);
    }

    // Pre-compute the nodal cardinality set
    for (size_t tetId = 0; tetId < boneMesh->getNumTetrahedra(); ++tetId)
    {
        const Vec4i& indices = boneMesh->getTetrahedronIndices(tetId);
        for (int i = 0; i < 4; i++)
        {
            m_nodalCardinalSet[indices[i]].push_back(tetId);
        }
    }
}

void
BoneDrillingCH::erodeBone()
{
    auto boneTetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(m_bone->getCollidingGeometry());

    ParallelUtils::parallelFor(m_colData->PColData.getSize(),
        [&](const size_t idx)
        {
            auto& cd = m_colData->PColData[idx];
            if (m_nodeRemovalStatus[cd.nodeIdx])
            {
                return;
            }

            m_nodalDensity[cd.nodeIdx] -= 0.001 * (m_angularSpeed / m_BoneHardness) * m_stiffness * cd.penetrationVector.norm() * 0.001;

            if (m_nodalDensity[cd.nodeIdx] <= 0.)
            {
                /// \todo Unused variable, maybe used in furture?
                // lock.lock();
                // m_erodedNodes.push_back(cd.nodeId);
                // lock.unlock();
                m_nodeRemovalStatus[cd.nodeIdx] = true;

                // tag the tetra that will be removed
                for (auto& tetId : m_nodalCardinalSet[cd.nodeIdx])
                {
                    boneTetMesh->setTetrahedraAsRemoved(static_cast<unsigned int>(tetId));
                }
            }
        });
}

void
BoneDrillingCH::processCollisionData()
{
    // Check if any collisions
    const auto devicePosition = m_drill->getCollidingGeometry()->getTranslation();
    if (m_colData->PColData.isEmpty())
    {
        // Set the visual object position same as the colliding object position
        m_drill->getVisualGeometry()->setTranslation(devicePosition);
        return;
    }

    // Update visual object position

    // Aggregate collision data
    Vec3d  t = Vec3d::Zero();
    double maxDepthSqr = MIN_D;
    for (size_t i = 0; i < m_colData->PColData.getSize(); ++i)
    {
        const auto& cd = m_colData->PColData[i];
        if (m_nodeRemovalStatus[cd.nodeIdx])
        {
            continue;
        }

        const auto dSqr = cd.penetrationVector.squaredNorm();
        if (dSqr > maxDepthSqr)
        {
            maxDepthSqr = dSqr;
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
    m_prevPos     = devicePosition;
}
}// iMSTK
