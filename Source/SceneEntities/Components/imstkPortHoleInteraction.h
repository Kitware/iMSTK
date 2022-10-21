/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkComponent.h"
#include "imstkMath.h"

#pragma once

namespace imstk
{
class Geometry;
class PbdConstraint;
class PbdObject;
class PbdRigidLineToPointConstraint;
class TaskNode;

///
/// \class PortHoleInteraction
///
/// \brief Defines the behaviour to constrain a PbdObject LineMesh or Capsule
/// to a fixed port hole location.
///
/// Alternatively a physical setup can be used when the hardware, space, and
/// registration is available.
///
class PortHoleInteraction : public SceneBehaviour
{
public:
    PortHoleInteraction(const std::string& name = "PortHoleInteraction");

    void init() override;

    ///
    /// \brief Get/Set the tool to be constrained
    ///@{
    std::shared_ptr<PbdObject> getTool() const { return m_toolObject; }
    void setTool(std::shared_ptr<PbdObject> toolObject);
    ///@}

    ///
    /// \brief Get/Set the tool geometry used for constraining
    ///@{
    std::shared_ptr<Geometry> getToolGeometry() const { m_toolGeom; }
    void setToolGeometry(std::shared_ptr<Geometry> toolGeom);
    ///@}

    ///
    /// \brief Get/Set the port hole location to constrain the geometry too
    ///@{
    const Vec3d& getPortHoleLocation() const { return m_portHoleLocation; }
    void setPortHoleLocation(const Vec3d& portHoleLocation) { m_portHoleLocation = portHoleLocation; }
    ///@}

    ///
    /// \brief Get/Set constraint compliance. This effects how stiff the constraint
    /// of the line to
    ///@{
    double getCompliance() const { return m_compliance; }
    void setCompliance(const double compliance) { m_compliance = compliance; }
///@}

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