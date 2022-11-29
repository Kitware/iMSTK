/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkSceneObject.h"

namespace imstk
{
class Geometry;
class GeometryMap;
class AbstractDynamicalModel;

///
/// \class DynamicObject
///
/// \brief Base class for scene objects that move and/or deform
///
class DynamicObject : public SceneObject // CollidingObject
{
public:
    ~DynamicObject() override = default;

    ///
    /// \brief Set/Get the geometry used for Physics computations
    ///@{
    std::shared_ptr<Geometry> getPhysicsGeometry() const { return m_physicsGeometry; }
    virtual void setPhysicsGeometry(std::shared_ptr<Geometry> geometry) { m_physicsGeometry = geometry; }
    ///@}

    ///
    /// \brief Set/Get the Physics-to-Collision map
    ///@{
    std::shared_ptr<GeometryMap> getPhysicsToCollidingMap() const { return m_physicsToCollidingGeomMap; }
    void setPhysicsToCollidingMap(std::shared_ptr<GeometryMap> map) { m_physicsToCollidingGeomMap = map; }
    ///@}

    ///
    /// \brief Set/Get the Physics-to-Visual map
    ///@{
    std::shared_ptr<GeometryMap> getPhysicsToVisualMap() const { return m_physicsToVisualGeomMap; }
    void setPhysicsToVisualMap(std::shared_ptr<GeometryMap> map) { m_physicsToVisualGeomMap = map; }
    ///@}

    ///
    /// \brief Set/Get dynamical model
    ///@{
    virtual std::shared_ptr<AbstractDynamicalModel> getDynamicalModel() const { return m_dynamicalModel; }
    virtual void setDynamicalModel(std::shared_ptr<AbstractDynamicalModel> dynaModel) { m_dynamicalModel = dynaModel; }
    ///@}

    ///
    /// \brief Returns the number of degree of freedom
    ///
    size_t getNumOfDOF() const;

    ///
    /// \brief Update the physics geometry and the apply the maps (if defined)
    ///
    void updateGeometries() final;

    ///
    /// \brief Update only the physics geometry and apply collision map
    ///
    virtual void updatePhysicsGeometry();

    ///
    /// \brief Initialize the scene object
    ///
    bool initialize() override;

    ///
    /// \brief Reset the dynamic object by reseting the respective DynamicalModel and Geometry
    ///
    void reset() override;

protected:
    ///
    /// \brief Setup connectivity of task graph
    ///
    void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;

    void postModifiedAll() override;

    DynamicObject(const std::string& name) : SceneObject(name) { }

    std::shared_ptr<AbstractDynamicalModel> m_dynamicalModel = nullptr; ///< Dynamical model
    std::shared_ptr<Geometry> m_physicsGeometry = nullptr;              ///< Geometry used for Physics

    // Maps
    std::shared_ptr<GeometryMap> m_physicsToCollidingGeomMap = nullptr; ///< Maps from Physics to collision geometry
    std::shared_ptr<GeometryMap> m_physicsToVisualGeomMap    = nullptr; ///< Maps from Physics to visual geometry
};
} // namespace imstk