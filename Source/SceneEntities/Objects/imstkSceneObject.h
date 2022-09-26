/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkEntity.h"
#include "imstkMacros.h"

#include <vector>

namespace imstk
{
class VisualModel;
class Geometry;
class TaskGraph;
class TaskNode;

///
/// \class SceneObject
///
/// \brief Base class for all scene objects. A scene object can optionally be visible and
/// collide with other scene objects. A object of the class is static.
///
class SceneObject : public Entity
{
public:
    SceneObject(const std::string& name = "SceneObject");
    ~SceneObject() override = default;

    IMSTK_TYPE_NAME(SceneObject)

    ///
    /// \brief Get the computational graph
    ///
    std::shared_ptr<TaskGraph> getTaskGraph() const { return m_taskGraph; }

    ///
    /// \brief Sets the visual geometry, adds (sets the first) VisualModel
    ///
    std::shared_ptr<Geometry> getVisualGeometry() const;
    void setVisualGeometry(std::shared_ptr<Geometry> geometry);

    ///
    /// \brief Get/add visual model
    ///
    std::shared_ptr<VisualModel> getVisualModel(const int index) const;
    void addVisualModel(std::shared_ptr<VisualModel> visualModel);
    void removeVisualModel(std::shared_ptr<VisualModel> visualModel);

    ///
    /// \brief Returns the computational node for updating
    ///
    std::shared_ptr<TaskNode> getUpdateNode() const { return m_updateNode; }

    ///
    /// \brief Returns the computational node for updating geometry
    ///
    std::shared_ptr<TaskNode> getUpdateGeometryNode() const { return m_updateGeometryNode; }

    ///
    /// \brief Update the SceneObject, called during scene update
    ///
    virtual void update() { }

    ///
    /// \brief Update the visuals, called before render
    ///
    virtual void visualUpdate() { }

    ///
    /// \brief Update the geometries
    ///
    virtual void updateGeometries() { }

    ///
    /// \brief Initialize the scene object
    ///
    virtual bool initialize() { return true; }

    ///
    /// \brief Initializes the edges of the SceneObject's computational graph
    ///
    void initGraphEdges();

    ///
    /// \brief
    ///
    virtual void reset() { }

protected:
    ///
    /// \brief Setup connectivity of the compute graph
    ///
    virtual void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink);

    ///
    /// \brief Posts modified for all geometries
    ///
    virtual void postModifiedAll();

    std::shared_ptr<TaskGraph> m_taskGraph = nullptr;         ///< Computational Graph

private:
    // Dissallow reassignment of these in subclasses
    std::shared_ptr<TaskNode> m_updateNode = nullptr;
    std::shared_ptr<TaskNode> m_updateGeometryNode = nullptr;
};
} // namespace imstk