/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#pragma once

#include "imstkVisualModel.h"
#include "imstkSceneEntity.h"

#include <string>
#include <vector>

namespace imstk
{
class ComputeNode;
class ComputeGraph;
class DeviceClient;
class Geometry;

///
/// \class SceneObject
///
/// \brief Base class for all scene objects. A scene object can optionally be visible and
/// collide with other scene objects. A object of the class is static.
///
class SceneObject : public SceneEntity
{
public:
    enum class Type
    {
        Visual,
        Animation,
        Colliding,
        Rigid,
        FEMDeformable,
        Pbd,
        SPH
    };

    ///
    /// \brief Constructor
    ///
    explicit SceneObject(const std::string& name);

    ///
    /// \brief Destructor
    ///
    virtual ~SceneObject() = default;

public:
    ///
    /// \brief Get the type of the object
    ///
    const Type& getType() const { return m_type; }

    ///
    /// \brief Get the computational graph
    ///
    std::shared_ptr<ComputeGraph> getComputeGraph() const { return m_computeGraph; }

    ///
    /// \brief Get/Set the custom name of the scene object
    ///
    const std::string& getName() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }

    ///
    /// \brief DEPRECATED: Get/Set geometry used for viewing
    ///
    /// Use getVisualModel() and addVisualModel() instead
    ///
    std::shared_ptr<Geometry> getVisualGeometry() const;
    void setVisualGeometry(std::shared_ptr<Geometry> geometry);

    ///
    /// \brief Get/add visual model
    ///
    std::shared_ptr<VisualModel> getVisualModel(unsigned int index) { return m_visualModels[index]; }
    void addVisualModel(std::shared_ptr<VisualModel> visualModel) { m_visualModels.push_back(visualModel); }

    ///
    /// \brief Get all visual models
    ///
    const std::vector<std::shared_ptr<VisualModel>>& getVisualModels() { return m_visualModels; }

    ///
    /// \brief Get number of visual models
    ///
    size_t getNumVisualModels() { return m_visualModels.size(); }

    ///
    /// \brief Get the master geometry
    ///
    virtual std::shared_ptr<Geometry> getMasterGeometry() const { return this->getVisualGeometry(); }

    ///
    /// \brief Returns the computational node for updating
    ///
    std::shared_ptr<ComputeNode> getUpdateNode() const { return m_updateNode; }

    ///
    /// \brief Returns the computational node for updating geometry
    ///
    std::shared_ptr<ComputeNode> getUpdateGeometryNode() const { return m_updateGeometryNode; }

    ///
    /// \brief
    ///
    virtual void update() { }

    ///
    /// \brief
    ///
    virtual void updateGeometries() {}

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
    virtual void initGraphEdges(std::shared_ptr<ComputeNode> source, std::shared_ptr<ComputeNode> sink);

protected:
    Type m_type;                                              ///> Type of the scene object
    std::string m_name;                                       ///> Custom name of the scene object
    std::vector<std::shared_ptr<VisualModel>> m_visualModels; ///> Visual objects for rendering
    std::shared_ptr<ComputeGraph> m_computeGraph = nullptr;   ///> Computational Graph

private:
    // Dissallow reassignment of these in subclasses
    std::shared_ptr<ComputeNode> m_updateNode = nullptr;
    std::shared_ptr<ComputeNode> m_updateGeometryNode = nullptr;
};

using VisualObject = SceneObject;
} // imstk
