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
#include "imstkParallelFor.h"
#include "imstkRbdConstraint.h"
#include "imstkRigidObject2.h"
#include "imstkTetrahedralMesh.h"

namespace imstk
{
void
BoneDrillingCH::setInputObjectDrill(std::shared_ptr<RigidObject2> drillObject)
{
    setInputObjectB(drillObject);
}

std::shared_ptr<RigidObject2>
BoneDrillingCH::getDrillObj() const
{
    return std::dynamic_pointer_cast<RigidObject2>(getInputObjectB());
}

void
BoneDrillingCH::erodeBone(
    const std::vector<CollisionElement>& elementsA,
    const std::vector<CollisionElement>& elementsB)
{
    auto boneTetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(getBoneObj()->getCollidingGeometry());

    // BoneDrillingCH process tetra-pointdirection elements
    ParallelUtils::parallelFor(elementsA.size(),
        [&](const size_t idx)
        {
            const CollisionElement& elementA = elementsA[idx];
            const CollisionElement& elementB = elementsB[idx];

            if ((elementB.m_type != CollisionElementType::PointDirection && elementB.m_type != CollisionElementType::PointIndexDirection)
                || elementA.m_type != CollisionElementType::CellIndex || elementA.m_element.m_CellIndexElement.cellType != IMSTK_TETRAHEDRON)
            {
                return;
            }
            // Currently ony supports CDs that report the cell id
            if (elementA.m_element.m_CellIndexElement.idCount != 1)
            {
                return;
            }

            const int tetIndex = elementA.m_element.m_CellIndexElement.ids[0];
            double depth       = 0.0;
            if (elementB.m_type == CollisionElementType::PointDirection)
            {
                depth = elementB.m_element.m_PointDirectionElement.penetrationDepth;
            }
            else if (elementB.m_type == CollisionElementType::PointIndexDirection)
            {
                depth = elementB.m_element.m_PointIndexDirectionElement.penetrationDepth;
            }

            if (m_nodeRemovalStatus[tetIndex])
            {
                return;
            }

            m_nodalDensity[tetIndex] -= 0.001 * (m_angularSpeed / m_BoneHardness) * m_stiffness * depth * 0.001;

            if (m_nodalDensity[tetIndex] <= 0.)
            {
                /// \todo Unused variable, maybe used in furture?
                // lock.lock();
                // m_erodedNodes.push_back(cd.nodeId);
                // lock.unlock();
                m_nodeRemovalStatus[tetIndex] = true;

                // tag the tetra that will be removed
                for (auto& tetId : m_nodalCardinalSet[tetIndex])
                {
                    boneTetMesh->setTetrahedraAsRemoved(static_cast<unsigned int>(tetId));
                }
            }
        });
}

void
BoneDrillingCH::handle(
    const std::vector<CollisionElement>& elementsA,
    const std::vector<CollisionElement>& elementsB)
{
    std::shared_ptr<CollidingObject> bone  = getBoneObj();
    std::shared_ptr<RigidObject2>    drill = getDrillObj();

    auto boneMesh = std::dynamic_pointer_cast<TetrahedralMesh>(bone->getCollidingGeometry());

    if (m_nodalDensity.size() != boneMesh->getNumVertices())
    {
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
        for (size_t tetId = 0; tetId < boneMesh->getNumCells(); ++tetId)
        {
            const Vec4i& indices = (*boneMesh->getCells())[tetId];
            for (int i = 0; i < 4; i++)
            {
                m_nodalCardinalSet[indices[i]].push_back(tetId);
            }
        }
    }

    // BoneDrillingCH uses both sides of collision data
    if (elementsA.size() != elementsB.size())
    {
        return;
    }

    // Check if any collisions
    const auto devicePosition = drill->getCollidingGeometry()->getTranslation();
    if (elementsA.empty() && elementsB.empty())
    {
        // Set the visual object position same as the colliding object position
        drill->getVisualGeometry()->setTranslation(devicePosition);
        return;
    }

    // Update visual object position

    // Aggregate collision data
    Vec3d  t = Vec3d::Zero();
    double maxDepthSqr = MIN_D;
    for (size_t i = 0; i < elementsB.size(); i++)
    {
        const CollisionElement& elementA = elementsA[i];
        const CollisionElement& elementB = elementsB[i];

        if ((elementB.m_type != CollisionElementType::PointDirection && elementB.m_type != CollisionElementType::PointIndexDirection)
            || elementA.m_type != CollisionElementType::CellIndex || elementA.m_element.m_CellIndexElement.cellType != IMSTK_TETRAHEDRON)
        {
            return;
        }
        // Currently ony supports CDs that report the cell id
        if (elementA.m_element.m_CellIndexElement.idCount != 1)
        {
            return;
        }

        const int tetIndex = elementA.m_element.m_CellIndexElement.ids[0];
        double    depth    = 0.0;
        Vec3d     dir      = Vec3d::Zero();
        if (elementB.m_type == CollisionElementType::PointDirection)
        {
            depth = elementB.m_element.m_PointDirectionElement.penetrationDepth;
            dir   = elementB.m_element.m_PointDirectionElement.dir;
        }
        else if (elementB.m_type == CollisionElementType::PointIndexDirection)
        {
            depth = elementB.m_element.m_PointIndexDirectionElement.penetrationDepth;
            dir   = elementB.m_element.m_PointIndexDirectionElement.dir;
        }

        if (m_nodeRemovalStatus[tetIndex])
        {
            continue;
        }

        const double dSqr = depth * depth;
        if (dSqr > maxDepthSqr)
        {
            maxDepthSqr = dSqr;
            t = dir;
        }
    }
    drill->getVisualGeometry()->setTranslation(devicePosition + t);

    // Spring force
    Vec3d force = m_stiffness * (drill->getVisualGeometry()->getTranslation() - devicePosition);

    // Damping force
    const double dt = 0.1; // Time step size to calculate the object velocity
    force += m_initialStep ? Vec3d(0.0, 0.0, 0.0) : m_damping * (devicePosition - m_prevPos) / dt;

    // Set external force on body
    (*drill->getRigidBody()->m_force) = force;

    // Decrease the density at the nodal points and remove if the density goes below 0
    this->erodeBone(elementsA, elementsB);

    // Housekeeping
    m_initialStep = false;
    m_prevPos     = devicePosition;
}
} // namespace imstk