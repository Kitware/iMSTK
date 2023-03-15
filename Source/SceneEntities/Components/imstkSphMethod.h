/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMacros.h"
#include "imstkComponent.h"

namespace imstk
{
class Geometry;
class GeometryMap;
class SphModel;

class SphMethod : public SceneBehaviour
{
public:
    SphMethod(const std::string& name = "SphMethod");

    IMSTK_TYPE_NAME(SphMethod)

    ///
    /// \brief Get the model governing the SPH fluid dynamics of this object
    ///
    imstkSetGetMacro(SphSystem, m_sphSystem, std::shared_ptr<SphModel>)

    ///
    /// \brief Set/Get the geometry used for Physics computations
    ///
    imstkSetGetMacro(Geometry, m_physicsGeometry, std::shared_ptr<Geometry>)

    ///
    /// \brief Set/Get the Physics-to-Collision map
    ///
    imstkSetGetMacro(PhysicsToCollidingMap, m_physicsToCollidingGeomMap, std::shared_ptr<GeometryMap>)

    ///
    /// \brief Set/Get the Physics-to-Visual map
    ///
    imstkSetGetMacro(PhysicsToVisualMap, m_physicsToVisualGeomMap, std::shared_ptr<GeometryMap>)

    ///
    /// \brief Returns the computational node for updating
    ///
    std::shared_ptr<TaskNode> getUpdateNode() const { return m_updateNode; }

    ///
    /// \brief Returns the computational node for updating geometry
    ///
    std::shared_ptr<TaskNode> getUpdateGeometryNode() const { return m_updateGeometryNode; }

    void updateGeometries();

    ///
    /// \brief Reset the dynamic object by reseting the respective DynamicalSystem and Geometry
    ///
    void reset();

private:
    void init() override;

    void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;

    void updatePhysicsGeometry();

    void postModifiedAll();

    std::shared_ptr<SphModel> m_sphSystem;

    // Physics related geometry and maps
    std::shared_ptr<Geometry>    m_physicsGeometry;           ///< Geometry used for Physics
    std::shared_ptr<GeometryMap> m_physicsToCollidingGeomMap; ///< Maps from Physics to collision geometry
    std::shared_ptr<GeometryMap> m_physicsToVisualGeomMap;    ///< Maps from Physics to visual geometry

    std::shared_ptr<TaskNode> m_updateNode;
    std::shared_ptr<TaskNode> m_updateGeometryNode;
};
} // namespace imstk
