/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPortHoleInteraction.h"
#include "imstkCapsule.h"
#include "imstkLineMesh.h"
#include "imstkPbdContactConstraint.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdSolver.h"
#include "imstkTaskGraph.h"
#include "imstkTaskNode.h"

namespace imstk
{
PortHoleInteraction::PortHoleInteraction(const std::string& name) : SceneBehaviour(true, name),
    m_portConstraint(std::make_shared<PbdRigidLineToPointConstraint>())
{
    m_portHoleHandleNode = std::make_shared<TaskNode>([&]()
        {
            handlePortHole();
        }, "PortHoleHandle");
    m_collisionGeometryUpdateNode = std::make_shared<TaskNode>([&]()
        {
            m_toolObject->updateGeometries();
        }, "CollisionGeometryUpdate");

    m_constraints.resize(1);
    m_constraints[0] = m_portConstraint.get();
}

void
PortHoleInteraction::init()
{
    CHECK(m_toolObject != nullptr) << "PortHoleInteraction requires a input tool object,"
        "please provide it with PortHoleInteraction::setTool";
    CHECK(m_toolGeom != nullptr) << "PortHoleInteraction requires a tool geometry,"
        "please provide it with PortHoleInteraction::setToolGeometry";

    m_taskGraph->addNode(m_portHoleHandleNode);
    m_taskGraph->addNode(m_collisionGeometryUpdateNode);
    m_taskGraph->addNode(m_toolObject->getPbdModel()->getIntegratePositionNode());
    m_taskGraph->addNode(m_toolObject->getPbdModel()->getSolveNode());
}

void
PortHoleInteraction::setTool(std::shared_ptr<PbdObject> toolObject)
{
    CHECK(m_toolObject == nullptr) << "PortHoleInteraction does not yet support changing"
        "the tool at runtime, please set before initialization of the scene";
    m_toolObject = toolObject;
}

void
PortHoleInteraction::setToolGeometry(std::shared_ptr<Geometry> toolGeom)
{
    CHECK(std::dynamic_pointer_cast<LineMesh>(toolGeom) != nullptr
        || std::dynamic_pointer_cast<Capsule>(toolGeom) != nullptr) <<
        "PortHoleInteraction only works with capsule or line tool geometry";
    m_toolGeom = toolGeom;
}

void
PortHoleInteraction::handlePortHole()
{
    // \todo: Extend to support constraint of non single element tools and non straight tools
    CHECK(m_toolGeom != nullptr) <<
        "PortHoleInteraction requires a tool geometry";

    Vec3d p, q;
    p = q = Vec3d::Zero();
    if (auto lineMesh = std::dynamic_pointer_cast<LineMesh>(m_toolGeom))
    {
        CHECK(lineMesh->getNumVertices() == 2) <<
            "PortHoleInteraction currently only works with straight single segment lines";

        p = (*lineMesh->getVertexPositions())[0];
        q = (*lineMesh->getVertexPositions())[1];
    }
    else if (auto capsule = std::dynamic_pointer_cast<Capsule>(m_toolGeom))
    {
        const Vec3d  capsule1Pos        = capsule->getPosition();
        const Vec3d  capsule1Axis       = capsule->getOrientation().toRotationMatrix().col(1).normalized();
        const double capsule1HalfLength = capsule->getLength() * 0.5;
        const Vec3d  diff1 = capsule1Axis * capsule1HalfLength;
        const double capsule1Radius = capsule->getRadius();

        p = capsule1Pos - diff1;
        q = capsule1Pos + diff1;
    }

    std::shared_ptr<PbdModel> pbdModel = m_toolObject->getPbdModel();
    const PbdParticleId       vid      = pbdModel->addVirtualParticle(m_portHoleLocation, 0.0);

    m_portConstraint->initConstraint(m_toolObject->getPbdModel()->getBodies(),
        { m_toolObject->getPbdBody()->bodyHandle, 0 },
                p, q, vid,
                m_compliance);

    pbdModel->getSolver()->addConstraints(&m_constraints);
}

void
PortHoleInteraction::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    // Add the collision constraint after internal solve but before collision solve
    // Also ensure collision geometry is up to date
    m_taskGraph->addEdge(source, m_toolObject->getPbdModel()->getSolveNode());

    m_taskGraph->addEdge(m_toolObject->getPbdModel()->getIntegratePositionNode(), m_collisionGeometryUpdateNode);
    m_taskGraph->addEdge(m_collisionGeometryUpdateNode, m_portHoleHandleNode);
    m_taskGraph->addEdge(m_portHoleHandleNode, m_toolObject->getPbdModel()->getSolveNode());

    m_taskGraph->addEdge(m_portHoleHandleNode, sink);
}
} // namespace imstk