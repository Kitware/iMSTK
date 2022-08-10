/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkSceneEntity.h"
#include "imstkMacros.h"

#include <memory>
#include <vector>

namespace imstk
{
class VisualModel;
class DeviceClient;
class Geometry;
class TaskGraph;
class TaskNode;

///
/// \class SceneObject
///
/// \brief Base class for all scene objects. A scene object can optionally be visible and
/// collide with other scene objects. A object of the class is static.
///
class SceneObject : public SceneEntity
{
public:
    SceneObject(const std::string& name = "SceneObject");
    ~SceneObject() override = default;

    IMSTK_TYPE_NAME(SceneObject)

    // *INDENT-OFF*
    SIGNAL(SceneObject, modified);
    // *INDENT-ON*

    ///
    /// \brief Get the computational graph
    ///
    std::shared_ptr<TaskGraph> getTaskGraph() const { return m_taskGraph; }

    ///
    /// \brief Get/Set the custom name of the scene object
    ///
    const std::string& getName() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }

    ///
    /// \brief Sets the visual geometry, adds (sets the first) VisualModel
    ///
    std::shared_ptr<Geometry> getVisualGeometry() const;
    void setVisualGeometry(std::shared_ptr<Geometry> geometry);

    ///
    /// \brief Get/add visual model
    ///
    std::shared_ptr<VisualModel> getVisualModel(unsigned int index) { return m_visualModels[index]; }
    void addVisualModel(std::shared_ptr<VisualModel> visualModel)
    {
        m_visualModels.push_back(visualModel);
        this->postEvent(Event(modified()));
    }

    void removeVisualModel(std::shared_ptr<VisualModel> visualModel)
    {
        auto iter = std::find(m_visualModels.begin(), m_visualModels.end(), visualModel);
        if (iter != m_visualModels.end())
        {
            m_visualModels.erase(iter);
        }
        this->postEvent(Event(modified()));
    }

    ///
    /// \brief Get all visual models
    ///
    const std::vector<std::shared_ptr<VisualModel>>& getVisualModels() { return m_visualModels; }

    ///
    /// \brief Get number of visual models
    ///
    size_t getNumVisualModels() { return m_visualModels.size(); }

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

    std::string m_name;                                       ///< Custom name of the scene object
    std::vector<std::shared_ptr<VisualModel>> m_visualModels; ///< Visual objects for rendering
    std::shared_ptr<TaskGraph> m_taskGraph = nullptr;         ///< Computational Graph

private:
    // Dissallow reassignment of these in subclasses
    std::shared_ptr<TaskNode> m_updateNode = nullptr;
    std::shared_ptr<TaskNode> m_updateGeometryNode = nullptr;
};
} // namespace imstk