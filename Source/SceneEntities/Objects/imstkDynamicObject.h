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

#include "imstkCollidingObject.h"

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
class DynamicObject : public CollidingObject
{
public:
    ///
    /// \brief Destructor
    ///
    virtual ~DynamicObject() = default;

    ///
    /// \brief Set/Get the geometry used for Physics computations
    ///
    std::shared_ptr<Geometry> getPhysicsGeometry() const { return m_physicsGeometry; }
    virtual void setPhysicsGeometry(std::shared_ptr<Geometry> geometry) { m_physicsGeometry = geometry; }

    ///
    /// \brief Get the master geometry
    ///
    virtual std::shared_ptr<Geometry> getMasterGeometry() const override { return m_physicsGeometry; }

    ///
    /// \brief Set/Get the Physics-to-Collision map
    ///
    std::shared_ptr<GeometryMap> getPhysicsToCollidingMap() const { return m_physicsToCollidingGeomMap; }
    void setPhysicsToCollidingMap(std::shared_ptr<GeometryMap> map) { m_physicsToCollidingGeomMap = map; }

    ///
    /// \brief Set/Get the Physics-to-Visual map
    ///
    std::shared_ptr<GeometryMap> getPhysicsToVisualMap() const { return m_physicsToVisualGeomMap; }
    void setPhysicsToVisualMap(std::shared_ptr<GeometryMap> map) { m_physicsToVisualGeomMap = map; }

    ///
    /// \brief Set/Get dynamical model
    ///
    virtual std::shared_ptr<AbstractDynamicalModel> getDynamicalModel() const { return m_dynamicalModel; }
    virtual void setDynamicalModel(std::shared_ptr<AbstractDynamicalModel> dynaModel) { m_dynamicalModel = dynaModel; }

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
    void updatePhysicsGeometry();

    ///
    /// \brief Initialize the scene object
    ///
    virtual bool initialize() override;

    ///
    /// \brief Reset the dynamic object by reseting the respective DynamicalModel and Geometry
    ///
    virtual void reset() override;

protected:
    ///
    /// \brief Setup connectivity of compute graph
    ///
    virtual void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;

protected:

    ///
    /// \brief Constructor
    ///
    explicit DynamicObject(const std::string& name) : CollidingObject(name) {}

    std::shared_ptr<AbstractDynamicalModel> m_dynamicalModel = nullptr; ///> Dynamical model
    std::shared_ptr<Geometry> m_physicsGeometry = nullptr;              ///> Geometry used for Physics

    // Maps
    std::shared_ptr<GeometryMap> m_physicsToCollidingGeomMap = nullptr; ///> Maps from Physics to collision geometry
    std::shared_ptr<GeometryMap> m_physicsToVisualGeomMap    = nullptr; ///> Maps from Physics to visual geometry
    bool m_updateVisualFromPhysicsGeometry = true;                      ///> Defines if visual is updated from colliding mapping or physics mapping
};
} // imstk
