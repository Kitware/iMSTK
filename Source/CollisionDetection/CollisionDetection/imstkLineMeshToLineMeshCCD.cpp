/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCollisionData.h"
#include "imstkEdgeEdgeCCDState.h"
#include "imstkLineMesh.h"
#include "imstkLineMeshToLineMeshCCD.h"

namespace imstk
{
LineMeshToLineMeshCCD::LineMeshToLineMeshCCD()
{
    setRequiredInputType<LineMesh>(0);
    setRequiredInputType<LineMesh>(1);

    m_prevA = std::make_shared<LineMesh>();
    m_prevB = std::make_shared<LineMesh>();

    // Add empty velocities and invMasses attribute arrays.
    // Required for proper construction of ismtk::MeshSide defined in PbdCollisionHandling.
    m_prevA->setVertexAttribute("Velocities", std::make_shared<VecDataArray<double, 3>>());
    m_prevB->setVertexAttribute("Velocities", std::make_shared<VecDataArray<double, 3>>());
    m_prevA->setVertexAttribute("InvMass", std::make_shared<VecDataArray<double, 3>>());
    m_prevB->setVertexAttribute("InvMass", std::make_shared<VecDataArray<double, 3>>());
}

namespace
{
///
/// \brief Copy LineMesh geometry information (points only)
/// from source to destionation object.
///
bool
copyPointSetGeometry(const Geometry* source, PointSet& dest)
{
    bool success = false;
    // Make sure its a LineMesh.
    if (auto sourceLineMeshPtr = dynamic_cast<const PointSet*>(source))
    {
        // Make sure it has a vertex buffer.
        if (auto srcPointsPtr = sourceLineMeshPtr->getVertexPositions())
        {
            *dest.getVertexPositions() = *srcPointsPtr;
            success = true;
        }
    }
    return success;
}
} // namespace

void
LineMeshToLineMeshCCD::updatePreviousTimestepGeometry(
    std::shared_ptr<const Geometry> geomA,
    std::shared_ptr<const Geometry> geomB)
{
    bool success = copyPointSetGeometry(geomA.get(), *m_prevA);
    success = success && copyPointSetGeometry(geomB.get(), *m_prevB);
    if (!success)
    {
        LOG(WARNING) << "Failed to make a copy of previous geometries.";
    }
}

void
LineMeshToLineMeshCCD::internalComputeCollision(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>* elementsA,
    std::vector<CollisionElement>* elementsB)
{
    // incomplete inputs, early return
    if (!geomA || !geomB || (!elementsA && !elementsB))
    {
        LOG(WARNING) << "Incomplete inputs: one of more of geomA, geomB, elementsA, or elementsB"
            " are nullptr when they shouldn't be."
            "\n Self-collision requries geomA == geomB.";
        return;
    }

    if (!m_prevA || !m_prevB)
    {
        LOG(WARNING) << "Invalid cache. Null pointers encountered.";
        return;
    }

    // Pass previous geometries to collisionData.
    getCollisionData()->prevGeomA = m_prevA;
    getCollisionData()->prevGeomB = m_prevB;

    // auto prevAPtr = m_prevA->getVertexPositions();
    // auto prevBPtr = m_prevB->getVertexPositions();
    const auto& prevA = *m_prevA->getVertexPositions().get();
    const auto& prevB = *m_prevB->getVertexPositions().get();

    auto meshA = std::dynamic_pointer_cast<const LineMesh>(geomA);
    auto meshB = std::dynamic_pointer_cast<const LineMesh>(geomB);

    if (meshA->getNumVertices() != prevA.size() || meshB->getNumVertices() != prevB.size())
    {
        LOG(WARNING) << "Invalid cache. Size of arrays do not match input.";
        return;
    }

    // Flag to identify two-body vs self collision
    const bool selfCollision = (meshA == meshB);

    // Get vertex and cell data for current time step
    std::shared_ptr<const VecDataArray<double, 3>> verticesPtrA = meshA->getVertexPositions();
    std::shared_ptr<const VecDataArray<double, 3>> verticesPtrB = meshB->getVertexPositions();

    if (!verticesPtrA || !verticesPtrB)
    {
        //TODO: throw appropriate error: "Vertex/Cell data not found for colliding LineMesh objects".
        return;
    }

    const VecDataArray<double, 3>& verticesA = *verticesPtrA;
    const VecDataArray<double, 3>& verticesB = *verticesPtrB;

    std::shared_ptr<VecDataArray<int, 2>> linesAPtr = meshA->getCells();
    const VecDataArray<int, 2>&           linesA    = *linesAPtr;
    std::shared_ptr<VecDataArray<int, 2>> linesBPtr = meshB->getCells();
    const VecDataArray<int, 2>&           linesB    = *linesBPtr;
    for (size_t i = 0; i < static_cast<size_t>(meshA->getNumCells()); ++i)
    {
        const Vec2i& cellA = linesA[i];
        size_t       j     = selfCollision ? i + 2 : 0;
        for (; j < static_cast<size_t>(meshB->getNumCells()); ++j)
        {
            // If performing self-collision, do not process self or immediate neighboring cells (lines).
            if (selfCollision && (std::max(i, j) - std::min(i, j) <= 1))
            {
                continue;
            }
            const Vec2i& cellB = linesB[j];

            EdgeEdgeCCDState currState(verticesA[cellA(0)], verticesA[cellA(1)], verticesB[cellB(0)], verticesB[cellB(1)]);
            EdgeEdgeCCDState prevState(prevA[cellA(0)], prevA[cellA(1)], prevB[cellB(0)], prevB[cellB(1)]);

            // Test for collision between current and previous timestep, and create collision info.
            double relativeTimeOfImpact = 0.0;
            int    collisionCase = EdgeEdgeCCDState::testCollision(prevState, currState, relativeTimeOfImpact);
            if (collisionCase != 0)
            {
                if (elementsA)
                {
                    CellIndexElement elemA;
                    elemA.cellType = IMSTK_EDGE;
                    elemA.idCount  = 2;
                    elemA.parentId = i; // line id
                    elemA.ids[0]   = cellA(0);
                    elemA.ids[1]   = cellA(1);
                    CollisionElement e(elemA);
                    e.m_ccdData = true;
                    elementsA->push_back(e);
                }
                if (elementsB)
                {
                    CellIndexElement elemB;
                    elemB.cellType = IMSTK_EDGE;
                    elemB.idCount  = 2;
                    elemB.parentId = j; // line id
                    elemB.ids[0]   = cellB(0);
                    elemB.ids[1]   = cellB(1);
                    CollisionElement e(elemB);
                    e.m_ccdData = true;
                    elementsB->push_back(e);
                }
            }
        }
    }
}

void
LineMeshToLineMeshCCD::computeCollisionDataAB(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsA,
    std::vector<CollisionElement>& elementsB)
{
    internalComputeCollision(geomA, geomB, &elementsA, &elementsB);
}

void
LineMeshToLineMeshCCD::computeCollisionDataA(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsA)
{
    internalComputeCollision(geomA, geomB, &elementsA, nullptr);
}

void
LineMeshToLineMeshCCD::computeCollisionDataB(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsB)
{
    internalComputeCollision(geomA, geomB, nullptr, &elementsB);
}
} // namespace imstk