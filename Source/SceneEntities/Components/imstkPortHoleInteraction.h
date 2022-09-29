/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkSceneObject.h"
#include "imstkMath.h"

#pragma once

namespace imstk
{
class LineMesh;
class PbdConstraint;
class PbdObject;
class PbdRigidLineToPointConstraint;
class TaskNode;

///
/// \class PortHoleInteraction
///
/// \brief Defines the behaviour to constrain a PbdObject needle to a fixed
/// port hole location.
///
class PortHoleInteraction : public SceneObject
{
public:
    PortHoleInteraction(std::shared_ptr<PbdObject> toolObject);

    void setToolGeometry(std::shared_ptr<Geometry> toolGeom);
    void setPortHoleLocation(const Vec3d& portHoleLocation) { m_portHoleLocation = portHoleLocation; }

    double getCompliance() const { return m_compliance; }
    void setCompliance(const double compliance) { m_compliance = compliance; }

protected:
    void handlePortHole();

    void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;

    std::shared_ptr<PbdObject> m_toolObject = nullptr;
    std::shared_ptr<Geometry>  m_toolGeom   = nullptr;
    Vec3d  m_portHoleLocation = Vec3d::Zero();
    double m_compliance       = 0.0001;

    std::shared_ptr<PbdRigidLineToPointConstraint> m_portConstraint = nullptr;
    std::vector<PbdConstraint*> m_constraints;

    std::shared_ptr<TaskNode> m_portHoleHandleNode = nullptr;
    std::shared_ptr<TaskNode> m_collisionGeometryUpdateNode = nullptr;
};
} // namespace imstk