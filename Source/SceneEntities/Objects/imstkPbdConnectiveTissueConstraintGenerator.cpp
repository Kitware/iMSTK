/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCollisionUtils.h"
#include "imstkPbdConnectiveTissueConstraintGenerator.h"
#include "imstkLineMesh.h"
#include "imstkPbdBaryPointToPointConstraint.h"
#include "imstkPbdConstraintFunctor.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkTriangleToTetMap.h"

#include "imstkProximitySurfaceSelector.h"
#include "imstkConnectiveStrandGenerator.h"

namespace imstk
{
void
PbdConnectiveTissueConstraintGenerator::connectLineToTetMesh(std::shared_ptr<PbdObject> pbdObj, PbdConstraintContainer& constraints)
{
    auto                         tetMesh  = std::dynamic_pointer_cast<TetrahedralMesh>(pbdObj->getPhysicsGeometry());
    std::shared_ptr<SurfaceMesh> surfMesh = tetMesh->extractSurfaceMesh();

    // Setup a map to figure out what tet the tri is from for attachment to the tet
    TriangleToTetMap triToTetMap;
    triToTetMap.setParentGeometry(tetMesh);
    triToTetMap.setChildGeometry(surfMesh);
    triToTetMap.setTolerance(m_tolerance);
    triToTetMap.compute();

    auto lineMesh = std::dynamic_pointer_cast<LineMesh>(m_connectiveStrandObj->getPhysicsGeometry());
    // Find all vertices of the line mesh that are coincident with the surface of mesh A
    int verticesConnected = 0;
    for (int vertId = 0; vertId < lineMesh->getNumVertices(); vertId++)
    {
        const Vec3d vertexPosition = lineMesh->getVertexPosition(vertId);
        double      minSqrDist     = IMSTK_FLOAT_MAX;

        int nearestTriangleId = -1;
        for (int triId = 0; triId < surfMesh->getNumCells(); triId++)
        {
            const Vec3d& x1 = surfMesh->getVertexPosition(surfMesh->getCells()->at(triId)[0]);
            const Vec3d& x2 = surfMesh->getVertexPosition(surfMesh->getCells()->at(triId)[1]);
            const Vec3d& x3 = surfMesh->getVertexPosition(surfMesh->getCells()->at(triId)[2]);

            int         ptOnTriangleCaseType;
            const Vec3d closestPtOnTri = CollisionUtils::closestPointOnTriangle(vertexPosition,
                x1, x2, x3, ptOnTriangleCaseType);

            const double sqrDist = (closestPtOnTri - vertexPosition).squaredNorm();
            if (sqrDist < minSqrDist)
            {
                minSqrDist = sqrDist;
                nearestTriangleId = triId;
            }
        }

        // If the vertex is not on the surface mesh, ignore it.
        if (minSqrDist > m_tolerance)
        {
            continue;
        }

        verticesConnected++;

        const int   tetId   = triToTetMap.getParentTetId(nearestTriangleId);
        const Vec4d weights = tetMesh->computeBarycentricWeights(tetId, vertexPosition);
        const int   objId   = pbdObj->getPbdBody()->bodyHandle;

        // Constraint between point on tet to the vertex
        auto                       vertToTetConstraint = std::make_shared<PbdBaryPointToPointConstraint>();
        std::vector<PbdParticleId> ptsA = {
            { objId, (*tetMesh->getCells())[tetId][0] },
            { objId, (*tetMesh->getCells())[tetId][1] },
            { objId, (*tetMesh->getCells())[tetId][2] },
            { objId, (*tetMesh->getCells())[tetId][3] } };

        std::vector<double> weightsA = { weights[0], weights[1], weights[2], weights[3] };

        // Ligament vertex end on the gallblader
        std::vector<PbdParticleId> ptsB     = { { m_connectiveStrandObj->getPbdBody()->bodyHandle, vertId } };
        std::vector<double>        weightsB = { 1.0 };
        vertToTetConstraint->initConstraint(ptsA, weightsA, ptsB, weightsB, 0.8, 0.8);
        constraints.addConstraint(vertToTetConstraint);
    }
}

void
PbdConnectiveTissueConstraintGenerator::connectLineToSurfMesh(
    std::shared_ptr<PbdObject> pbdObj,
    PbdConstraintContainer&    constraints)
{
    auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(pbdObj->getPhysicsGeometry());
    auto lineMesh = std::dynamic_pointer_cast<LineMesh>(m_connectiveStrandObj->getPhysicsGeometry());

    // Find all vertices of the line mesh that are coincident with the surface of mesh A
    int verticesConnected = 0;
    for (int vertId = 0; vertId < lineMesh->getNumVertices(); vertId++)
    {
        const Vec3d vertexPosition = lineMesh->getVertexPosition(vertId);
        double      minSqrDist     = IMSTK_FLOAT_MAX;

        int nearestTriangleId = -1;
        for (int triId = 0; triId < surfMesh->getNumCells(); triId++)
        {
            const Vec3d& x1 = surfMesh->getVertexPosition(surfMesh->getCells()->at(triId)[0]);
            const Vec3d& x2 = surfMesh->getVertexPosition(surfMesh->getCells()->at(triId)[1]);
            const Vec3d& x3 = surfMesh->getVertexPosition(surfMesh->getCells()->at(triId)[2]);

            int         ptOnTriangleCaseType;
            const Vec3d closestPtOnTri = CollisionUtils::closestPointOnTriangle(vertexPosition,
                x1, x2, x3, ptOnTriangleCaseType);

            const double sqrDist = (closestPtOnTri - vertexPosition).squaredNorm();
            if (sqrDist < minSqrDist)
            {
                minSqrDist = sqrDist;
                nearestTriangleId = triId;
            }
        }

        // If the vertex is not on the surface mesh, ignore it.
        if (minSqrDist > m_tolerance)
        {
            continue;
        }

        verticesConnected++;

        const Vec3d weights = surfMesh->computeBarycentricWeights(nearestTriangleId, vertexPosition);
        const int   objId   = pbdObj->getPbdBody()->bodyHandle;

        // Constraint between point on surface triangle to the vertex
        auto                       vertToTriConstraint = std::make_shared<PbdBaryPointToPointConstraint>();
        std::vector<PbdParticleId> ptsA = {
            { objId, (*surfMesh->getCells())[nearestTriangleId][0] },
            { objId, (*surfMesh->getCells())[nearestTriangleId][1] },
            { objId, (*surfMesh->getCells())[nearestTriangleId][2] } };

        std::vector<double> weightsA = { weights[0], weights[1], weights[2] };

        // Ligament vertex end on the gallblader
        std::vector<PbdParticleId> ptsB     = { { m_connectiveStrandObj->getPbdBody()->bodyHandle, vertId } };
        std::vector<double>        weightsB = { 1.0 };
        vertToTriConstraint->initConstraint(ptsA, weightsA, ptsB, weightsB, 0.8, 0.8);
        constraints.addConstraint(vertToTriConstraint);
    }
}

void
PbdConnectiveTissueConstraintGenerator::generateDistanceConstraints()
{
    m_connectiveStrandObj->getPbdModel()->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, m_distStiffness,
        m_connectiveStrandObj->getPbdBody()->bodyHandle);
}

void
PbdConnectiveTissueConstraintGenerator::operator()(PbdConstraintContainer& constraints)
{
    auto objAPhysMeshSurf = std::dynamic_pointer_cast<SurfaceMesh>(m_objA->getPhysicsGeometry());
    if (objAPhysMeshSurf != nullptr)
    {
        connectLineToSurfMesh(m_objA, constraints);
    }

    auto objAPhysMeshTet = std::dynamic_pointer_cast<TetrahedralMesh>(m_objA->getPhysicsGeometry());
    if (objAPhysMeshTet != nullptr)
    {
        connectLineToTetMesh(m_objA, constraints);
    }

    auto objBPhysMeshSurf = std::dynamic_pointer_cast<SurfaceMesh>(m_objB->getPhysicsGeometry());
    if (objBPhysMeshSurf != nullptr)
    {
        connectLineToSurfMesh(m_objB, constraints);
    }

    auto objBPhysMeshTet = std::dynamic_pointer_cast<TetrahedralMesh>(m_objB->getPhysicsGeometry());
    if (objBPhysMeshTet != nullptr)
    {
        connectLineToTetMesh(m_objB, constraints);
    }
}

std::shared_ptr<PbdObject>
addConnectiveTissueConstraints(
    std::shared_ptr<LineMesh>  connectiveLineMesh,
    std::shared_ptr<PbdObject> objA,
    std::shared_ptr<PbdObject> objB,
    std::shared_ptr<PbdModel>  model)
{
    // Check inputs
    CHECK(connectiveLineMesh != nullptr) << "NULL line mesh passes to generateConnectiveTissueConstraints";
    CHECK(objA != nullptr) << "PbdObject objA is NULL in generateConnectiveTissueConstraints";
    CHECK(objB != nullptr) << "PbdObject objB is NULL in generateConnectiveTissueConstraints";

    auto connectiveStrands = std::make_shared<PbdObject>("connectiveTissue");

    // Setup the Object
    connectiveStrands->setVisualGeometry(connectiveLineMesh);
    connectiveStrands->setPhysicsGeometry(connectiveLineMesh);
    connectiveStrands->setCollidingGeometry(connectiveLineMesh);
    connectiveStrands->setDynamicalModel(model);

    double mass = 0.01;
    connectiveStrands->getPbdBody()->uniformMassValue = mass / connectiveLineMesh->getNumVertices();

    // Setup constraints between the gallblader and ligaments
    auto attachmentConstraintFunctor = std::make_shared<PbdConnectiveTissueConstraintGenerator>();
    attachmentConstraintFunctor->setConnectiveStrandObj(connectiveStrands);
    attachmentConstraintFunctor->generateDistanceConstraints();
    attachmentConstraintFunctor->setConnectedObjA(objA);
    attachmentConstraintFunctor->setConnectedObjB(objB);
    attachmentConstraintFunctor->setBodyIndex(connectiveStrands->getPbdBody()->bodyHandle);
    model->getConfig()->addPbdConstraintFunctor(attachmentConstraintFunctor);

    return connectiveStrands;
}

std::shared_ptr<PbdObject>
makeConnectiveTissue(
    std::shared_ptr<PbdObject>                objA,
    std::shared_ptr<PbdObject>                objB,
    std::shared_ptr<PbdModel>                 model,
    double                                    maxDist,
    double                                    strandsPerFace,
    int                                       segmentsPerStrand,
    std::shared_ptr<ProximitySurfaceSelector> proxSelector)
{
    proxSelector = std::make_shared<ProximitySurfaceSelector>();

    // Check inputs
    auto objASurf = std::dynamic_pointer_cast<SurfaceMesh>(objA->getCollidingGeometry());
    CHECK(objASurf != nullptr) << "Object A " << objA->getName() << " Did not contain a surface mesh as colliding geometry in generateConnectiveTissue";

    auto objBSurf = std::dynamic_pointer_cast<SurfaceMesh>(objB->getCollidingGeometry());
    CHECK(objBSurf != nullptr) << "Object B " << objB->getName() << " Did not contain a surface mesh as colliding geometry in generateConnectiveTissue";

    CHECK(model != nullptr) << "PbdModel in generateConnectiveTissue is NULL";

    Vec3d objACenter = std::dynamic_pointer_cast<SurfaceMesh>(objA->getCollidingGeometry())->getCenter();
    Vec3d objBCenter = std::dynamic_pointer_cast<SurfaceMesh>(objB->getCollidingGeometry())->getCenter();

    if (fabs(maxDist) < 1.0e-6)
    {
        maxDist = (objACenter - objBCenter).norm();
    }

    proxSelector->setInputMeshes(
        std::dynamic_pointer_cast<SurfaceMesh>(objA->getCollidingGeometry()),
        std::dynamic_pointer_cast<SurfaceMesh>(objB->getCollidingGeometry()));

    proxSelector->setProximity(maxDist);
    proxSelector->update();

    // Create surface connector to generate geometry of connective tissue
    auto surfConnector = std::make_shared<ConnectiveStrandGenerator>();
    surfConnector->setInputMeshes(
        std::dynamic_pointer_cast<SurfaceMesh>(proxSelector->getOutput(0)),
        std::dynamic_pointer_cast<SurfaceMesh>(proxSelector->getOutput(1)));
    surfConnector->setSegmentsPerStrand(segmentsPerStrand);
    surfConnector->setStrandsPerFace(strandsPerFace);
    surfConnector->update();

    // Get mesh for connective strands
    auto connectiveLineMesh = std::dynamic_pointer_cast<LineMesh>(surfConnector->getOutput(0));

    // Create PBD object of connective strands with associated constraints
    auto connectiveStrands = addConnectiveTissueConstraints(
        connectiveLineMesh, objA, objB, model);

    return connectiveStrands;
}
} // namespace imstk