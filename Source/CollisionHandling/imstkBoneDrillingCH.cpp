/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkBoneDrillingCH.h"
#include "imstkCollisionData.h"
#include "imstkCollider.h"
#include "imstkParallelFor.h"
#include "imstkPbdObject.h"
#include "imstkTetrahedralMesh.h"

namespace imstk
{
void
BoneDrillingCH::setInputObjectDrill(std::shared_ptr<PbdObject> drillObject)
{
    setInputObjectB(drillObject);
}

std::shared_ptr<PbdObject>
BoneDrillingCH::getDrillObj() const
{
    return std::dynamic_pointer_cast<PbdObject>(getInputObjectB());
}

void
BoneDrillingCH::erodeBone(
    std::shared_ptr<TetrahedralMesh>     boneMesh,
    const std::vector<CollisionElement>& elementsA,
    const std::vector<CollisionElement>& elementsB)
{
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
                    boneMesh->setTetrahedraAsRemoved(static_cast<unsigned int>(tetId));
                }
            }
        });
}

void
BoneDrillingCH::handle(
    const std::vector<CollisionElement>& elementsA,
    const std::vector<CollisionElement>& elementsB)
{
    std::shared_ptr<PbdObject> drill = getDrillObj();

    // Cache the tet mesh geometry pointer of the boneObj on the first call,
    // re-use the cached pointer in later iterations.
    if (!m_boneMesh)
    {
        m_boneMesh = std::dynamic_pointer_cast<TetrahedralMesh>(Collider::getCollidingGeometryFromEntity(getBoneObj().get()));
    }
    CHECK(m_boneMesh != nullptr) << "Cannot acquire shared_ptr to boneMesh.";

    if (m_nodalDensity.size() != m_boneMesh->getNumVertices())
    {
        // Initialize bone density values
        m_nodalDensity.reserve(m_boneMesh->getNumVertices());
        for (size_t i = 0; i < m_boneMesh->getNumVertices(); ++i)
        {
            m_nodalDensity.push_back(m_initialBoneDensity);
        }

        m_nodeRemovalStatus.reserve(m_boneMesh->getNumVertices());
        for (size_t i = 0; i < m_boneMesh->getNumVertices(); ++i)
        {
            m_nodeRemovalStatus.push_back(false);
        }

        m_nodalCardinalSet.reserve(m_boneMesh->getNumVertices());
        for (size_t i = 0; i < m_boneMesh->getNumVertices(); ++i)
        {
            std::vector<size_t> row;
            m_nodalCardinalSet.push_back(row);
        }

        // Pre-compute the nodal cardinality set
        for (size_t tetId = 0; tetId < m_boneMesh->getNumCells(); ++tetId)
        {
            const Vec4i& indices = (*m_boneMesh->getCells())[tetId];
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
    // Cache the collider geometry pointer of the drill object on the first call,
    // re-use the cached pointer in later iterations.
    if (!m_drillCollidingGeometry)
    {
        m_drillCollidingGeometry = Collider::getCollidingGeometryFromEntity(drill.get());
    }
    CHECK(m_drillCollidingGeometry != nullptr) << "Cannot acquire shared_ptr to drill colliding geometry.";
    const auto devicePosition = m_drillCollidingGeometry->getTranslation();
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
    drill->getPbdBody()->externalForce = force;

    // Decrease the density at the nodal points and remove if the density goes below 0
    this->erodeBone(m_boneMesh, elementsA, elementsB);

    // Housekeeping
    m_initialStep = false;
    m_prevPos     = devicePosition;
}
} // namespace imstk