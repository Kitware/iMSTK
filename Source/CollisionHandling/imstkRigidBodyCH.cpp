/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkRigidBodyCH.h"
#include "imstkCollider.h"
#include "imstkCollisionData.h"
#include "imstkCollisionUtils.h"
#include "imstkLineMesh.h"
#include "imstkRbdContactConstraint.h"
#include "imstkRbdFrictionConstraint.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObject2.h"
#include "imstkSurfaceMesh.h"
#include "imstkVecDataArray.h"

namespace imstk
{
void
RigidBodyCH::setInputRigidObjectA(std::shared_ptr<RigidObject2> rbdObjA)
{
    setInputObjectA(rbdObjA);
}

void
RigidBodyCH::setInputRigidObjectB(std::shared_ptr<RigidObject2> rbdObjB)
{
    setInputObjectB(rbdObjB);
}

void
RigidBodyCH::setInputCollidingObjectB(std::shared_ptr<Entity> colObjB)
{
    setInputObjectB(colObjB);
}

std::shared_ptr<RigidObject2>
RigidBodyCH::getRigidObjA()
{
    return std::dynamic_pointer_cast<RigidObject2>(getInputObjectA());
}

std::shared_ptr<RigidObject2>
RigidBodyCH::getRigidObjB()
{
    return std::dynamic_pointer_cast<RigidObject2>(getInputObjectB());
}

void
RigidBodyCH::handle(
    const std::vector<CollisionElement>& elementsA,
    const std::vector<CollisionElement>& elementsB)
{
    std::shared_ptr<RigidObject2> rbdObjA = getRigidObjA();
    std::shared_ptr<RigidObject2> rbdObjB = getRigidObjB();
    std::shared_ptr<Entity>       colObjB = getInputObjectB();

    // If both objects are rigid objects
    if (rbdObjA != nullptr && rbdObjB != nullptr)
    {
        // If we only have elements of A, process one-sided rigid
        if (elementsB.size() == 0 && elementsA.size() != 0)
        {
            handleRbdStaticOneWay(rbdObjA, colObjB, elementsA, elementsB);
        }
        // If we have both elements
        else
        {
            std::shared_ptr<RigidBodyModel2> rbdModelA = rbdObjA->getRigidBodyModel2();
            std::shared_ptr<RigidBodyModel2> rbdModelB = rbdObjB->getRigidBodyModel2();

            // If the two bodies exist in separate rbd models, add two one-way constraints
            // to each system
            if (rbdModelA != rbdModelB)
            {
                if (rbdModelA != nullptr)
                {
                    handleRbdStaticOneWay(rbdObjA, nullptr, elementsA, elementsB);
                }
                if (rbdModelB != nullptr)
                {
                    handleRbdStaticOneWay(rbdObjB, nullptr, elementsB, elementsA);
                }
            }
            // If in the same model use one two-way constraint
            else
            {
                handleRbdRbdTwoWay(rbdObjA, rbdObjB, elementsA, elementsB);
            }
        }
    }
    // If objA is rigid and b is colliding
    else if (rbdObjA != nullptr && colObjB != nullptr)
    {
        // Process one sided with both
        handleRbdStaticOneWay(rbdObjA, colObjB, elementsA, elementsB);
    }
}

void
RigidBodyCH::handleRbdRbdTwoWay(
    std::shared_ptr<RigidObject2>        rbdObjA,
    std::shared_ptr<RigidObject2>        rbdObjB,
    const std::vector<CollisionElement>& elementsA,
    const std::vector<CollisionElement>& elementsB)
{
    if (elementsA.size() != elementsB.size())
    {
        return;
    }

    auto geom = std::dynamic_pointer_cast<PointSet>(Collider::getCollidingGeometryFromEntity(rbdObjA.get()));

    // Generate one two-way constraint
    std::shared_ptr<RigidBodyModel2> rbdModelAB = rbdObjA->getRigidBodyModel2();
    for (size_t i = 0; i < elementsA.size(); i++)
    {
        const CollisionElement& colElemA = elementsA[i];
        if (colElemA.m_type == CollisionElementType::PointDirection)
        {
            const Vec3d& dir       = colElemA.m_element.m_PointDirectionElement.dir;
            const double depth     = colElemA.m_element.m_PointDirectionElement.penetrationDepth;
            const Vec3d& contactPt = colElemA.m_element.m_PointDirectionElement.pt;

            addConstraint(rbdObjA, rbdObjB, contactPt, dir, depth);
        }
        else if (colElemA.m_type == CollisionElementType::PointIndexDirection)
        {
            // Doesn't support mapping yet
            const Vec3d& dir       = colElemA.m_element.m_PointIndexDirectionElement.dir;
            const double depth     = colElemA.m_element.m_PointIndexDirectionElement.penetrationDepth;
            const Vec3d& contactPt = (*geom->getVertexPositions())[colElemA.m_element.m_PointIndexDirectionElement.ptIndex];

            addConstraint(rbdObjA, rbdObjB, contactPt, dir, depth);
        }
    }
}

void
RigidBodyCH::handleRbdStaticOneWay(
    std::shared_ptr<RigidObject2>        rbdObj,
    std::shared_ptr<Entity>              colObj,
    const std::vector<CollisionElement>& elementsA,
    const std::vector<CollisionElement>& elementsB)
{
    // First handle one-way point-direction constraints
    for (size_t i = 0; i < elementsA.size(); i++)
    {
        const CollisionElement& colElem = elementsA[i];
        if (colElem.m_type == CollisionElementType::PointDirection)
        {
            const Vec3d& dir       = colElem.m_element.m_PointDirectionElement.dir;
            const double depth     = colElem.m_element.m_PointDirectionElement.penetrationDepth;
            const Vec3d& contactPt = colElem.m_element.m_PointDirectionElement.pt;

            addConstraint(rbdObj, contactPt, dir, depth);
        }
        else if (colElem.m_type == CollisionElementType::PointIndexDirection)
        {
            // Doesn't support mapping yet
            auto         geom      = std::dynamic_pointer_cast<PointSet>(Collider::getCollidingGeometryFromEntity(rbdObj.get()));
            const Vec3d& dir       = colElem.m_element.m_PointIndexDirectionElement.dir;
            const double depth     = colElem.m_element.m_PointIndexDirectionElement.penetrationDepth;
            const Vec3d& contactPt = (*geom->getVertexPositions())[colElem.m_element.m_PointIndexDirectionElement.ptIndex];

            addConstraint(rbdObj, contactPt, dir, depth);
        }
    }

    // So long as both sides were filled we may have two-way
    if (elementsA.size() != elementsB.size() || colObj == nullptr)
    {
        return;
    }

    // Two-way is only supported through mesh-mesh
    auto geomA = std::dynamic_pointer_cast<PointSet>(Collider::getCollidingGeometryFromEntity(rbdObj.get()));
    auto geomB = std::dynamic_pointer_cast<PointSet>(Collider::getCollidingGeometryFromEntity(colObj.get()));
    if (geomA == nullptr || geomB == nullptr)
    {
        return;
    }

    std::shared_ptr<VecDataArray<double, 3>> verticesAPtr = geomA->getVertexPositions();
    const VecDataArray<double, 3>&           verticesA    = *verticesAPtr;
    std::shared_ptr<VecDataArray<double, 3>> verticesBPtr = geomB->getVertexPositions();
    const VecDataArray<double, 3>&           verticesB    = *verticesBPtr;

    // Generate one two-way constraint
    std::shared_ptr<RigidBodyModel2> rbdModelA = rbdObj->getRigidBodyModel2();
    for (size_t i = 0; i < elementsA.size(); i++)
    {
        const CollisionElement& colElemA = elementsA[i];
        const CollisionElement& colElemB = elementsB[i];

        // Only handle mesh-mesh constraints here
        if (colElemA.m_type != CollisionElementType::CellIndex || colElemB.m_type != CollisionElementType::CellIndex)
        {
            continue;
        }

        const CellIndexElement& elemA = colElemA.m_element.m_CellIndexElement;
        const CellIndexElement& elemB = colElemB.m_element.m_CellIndexElement;

        // Vertex vs Triangle
        if (elemA.cellType == IMSTK_VERTEX && elemB.cellType == IMSTK_TRIANGLE)
        {
            const Vec3d& p = verticesA[elemA.ids[0]];

            Vec3i tri = Vec3i::Zero();
            if (elemB.idCount == 1)
            {
                tri = (*dynamic_cast<SurfaceMesh*>(geomB.get())->getCells().get())[elemB.ids[0]];
            }
            else if (elemB.idCount == 3)
            {
                tri = Vec3i(elemB.ids[0], elemB.ids[1], elemB.ids[2]);
            }
            const Vec3d& a = verticesB[tri[0]];
            const Vec3d& b = verticesB[tri[1]];
            const Vec3d& c = verticesB[tri[2]];

            // Project the vertex onto the triangle
            Vec3d  n;
            double depth;
            {
                const Vec3d v0 = b - a;
                const Vec3d v1 = c - a;
                const Vec3d v2 = p - a;
                n     = v0.cross(v1).normalized();
                depth = v2.dot(n);
            }
            Vec3d contactPt = p + n * depth; // Point n triangle

            addConstraint(rbdObj, contactPt, n, depth);
        }
        // Edge vs Edge
        else if (elemA.cellType == IMSTK_EDGE && elemB.cellType == IMSTK_EDGE)
        {
            Vec2i edgeA = Vec2i::Zero();
            if (elemA.idCount == 1)
            {
                edgeA = (*dynamic_cast<LineMesh*>(geomA.get())->getCells().get())[elemA.ids[0]];
            }
            else if (elemA.idCount == 2)
            {
                edgeA = Vec2i(elemA.ids[0], elemA.ids[1]);
            }
            Vec2i edgeB = Vec2i::Zero();
            if (elemB.idCount == 1)
            {
                edgeB = (*dynamic_cast<LineMesh*>(geomB.get())->getCells().get())[elemB.ids[0]];
            }
            else if (elemB.idCount == 2)
            {
                edgeB = Vec2i(elemB.ids[0], elemB.ids[1]);
            }

            // Measure closest distances
            Vec3d pA, pB;
            CollisionUtils::edgeToEdgeClosestPoints(
                verticesA[edgeA[0]], verticesA[edgeA[1]],
                verticesB[edgeB[0]], verticesB[edgeB[1]], pA, pB);

            const Vec3d  diff = pB - pA;
            const double l    = diff.norm();
            if (l > 0.0)
            {
                // If A is within/behind edge B, then pB-pA gives direction to move A out of B
                const Vec3d n = diff / l;

                // pA is point of contact on objA which is rigid body
                addConstraint(rbdObj, pA, n, l);
            }
        }
        // Edge vs Vertex
        else if (elemA.cellType == IMSTK_EDGE && elemB.cellType == IMSTK_VERTEX)
        {
            Vec2i edge = Vec2i::Zero();
            if (elemA.idCount == 1)
            {
                edge = (*dynamic_cast<LineMesh*>(geomA.get())->getCells().get())[elemA.ids[0]];
            }
            else if (elemA.idCount == 2)
            {
                edge = Vec2i(elemA.ids[0], elemA.ids[1]);
            }
            const Vec3d& a = verticesA[edge[0]];
            const Vec3d& b = verticesA[edge[1]];

            const Vec3d& pt = verticesB[elemB.ids[0]];

            const Vec3d  ab     = b - a;
            const double length = ab.norm();
            const Vec3d  dir1   = ab / length;

            // Project onto the line
            const Vec3d  diff = pt - a;
            const double p    = dir1.dot(diff);

            // Remove tangent component to get normal
            const Vec3d  diff1 = diff - p * dir1;
            const double l     = diff1.norm();
            if (l > 0.0)
            {
                const Vec3d n = diff1 / l;
                const Vec3d contactPt = pt - n * l;
                addConstraint(rbdObj, contactPt, n, l);
            }
        }
        else if (elemA.cellType == IMSTK_VERTEX && elemB.cellType == IMSTK_EDGE)
        {
            const Vec3d& pt = verticesA[elemA.ids[0]];

            Vec2i edge = Vec2i::Zero();
            if (elemB.idCount == 1)
            {
                edge = (*dynamic_cast<LineMesh*>(geomB.get())->getCells().get())[elemB.ids[0]];
            }
            else if (elemB.idCount == 2)
            {
                edge = Vec2i(elemB.ids[0], elemB.ids[1]);
            }
            const Vec3d& a = verticesB[edge[0]];
            const Vec3d& b = verticesB[edge[1]];

            const Vec3d  ab     = b - a;
            const double length = ab.norm();
            const Vec3d  dir1   = ab / length;

            // Project onto the line
            const Vec3d  diff = pt - a;
            const double p    = dir1.dot(diff);

            // Remove tangent component to get normal
            const Vec3d  diff1 = diff - p * dir1;
            const double l     = diff1.norm();
            if (l > 0.0)
            {
                const Vec3d n = diff1 / l;
                const Vec3d contactPt = pt + n * l;
                addConstraint(rbdObj, contactPt, -n, l);
            }
        }
        else if (elemA.cellType == IMSTK_VERTEX && elemB.cellType == IMSTK_VERTEX)
        {
            const Vec3d& a = verticesA[elemA.ids[0]];  // Vertex to resolve
            const Vec3d& b = verticesB[elemB.ids[0]];

            const Vec3d  diff = b - a;
            const double l    = diff.norm();
            if (l > 0.0)
            {
                addConstraint(rbdObj, a, diff / l, l);
            }
        }
    }
}

void
RigidBodyCH::addConstraint(
    std::shared_ptr<RigidObject2> rbdObj,
    const Vec3d& contactPt, const Vec3d& contactNormal,
    const double contactDepth)
{
    auto contactConstraint = std::make_shared<RbdContactConstraint>(
            rbdObj->getRigidBody(), nullptr,
            contactNormal.normalized(), contactPt, contactDepth,
            m_beta,
            RbdConstraint::Side::A);
    contactConstraint->compute(rbdObj->getRigidBodyModel2()->getTimeStep());
    rbdObj->getRigidBodyModel2()->addConstraint(contactConstraint);

    if (m_useFriction)
    {
        std::shared_ptr<RbdFrictionConstraint> frictionConstraint =
            std::make_shared<RbdFrictionConstraint>(
                rbdObj->getRigidBody(), nullptr,
                contactPt, contactNormal.normalized(), contactDepth,
                m_frictionalCoefficient,
                RbdConstraint::Side::A);
        frictionConstraint->compute(rbdObj->getRigidBodyModel2()->getTimeStep());
        rbdObj->getRigidBodyModel2()->addConstraint(frictionConstraint);
    }
}

void
RigidBodyCH::addConstraint(
    std::shared_ptr<RigidObject2> rbdObjA,
    std::shared_ptr<RigidObject2> rbdObjB,
    const Vec3d& contactPt, const Vec3d& contactNormal,
    const double contactDepth)
{
    // Add a two-way constraint to solve both with one constraint
    if (rbdObjA->getRigidBodyModel2() == rbdObjB->getRigidBodyModel2())
    {
        auto contactConstraint = std::make_shared<RbdContactConstraint>(
            rbdObjA->getRigidBody(), rbdObjB->getRigidBody(),
            contactNormal.normalized(), contactPt, contactDepth,
            m_beta);
        contactConstraint->compute(rbdObjA->getRigidBodyModel2()->getTimeStep());
        rbdObjA->getRigidBodyModel2()->addConstraint(contactConstraint);

        if (m_useFriction)
        {
            auto frictionConstraint = std::make_shared<RbdFrictionConstraint>(
                    rbdObjA->getRigidBody(), rbdObjB->getRigidBody(),
                    contactPt, contactNormal.normalized(), contactDepth,
                    m_frictionalCoefficient,
                    RbdConstraint::Side::AB);
            frictionConstraint->compute(rbdObjA->getRigidBodyModel2()->getTimeStep());
            rbdObjA->getRigidBodyModel2()->addConstraint(frictionConstraint);
        }
    }
    // If both belong to differing systems then use two one-way constraints
    else
    {
        addConstraint(rbdObjA, contactPt, contactNormal, contactDepth);
        addConstraint(rbdObjB, contactPt, -contactNormal, contactDepth);
    }
}
} // namespace imstk