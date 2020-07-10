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

#include "imstkSceneObject.h"
#include "imstkMath.h"

namespace imstk
{
class Geometry;
class GeometryMap;

/// \brief TODO
class CollidingObject : public SceneObject
{
public:
    ///
    /// \brief
    ///
    explicit CollidingObject(const std::string& name) : SceneObject(name)
    {
        m_type = Type::Colliding;
    }

    ///
    /// \brief
    ///
    virtual ~CollidingObject() override = default;

    ///
    /// \brief
    ///
    std::shared_ptr<Geometry> getCollidingGeometry() const;
    void setCollidingGeometry(const std::shared_ptr<Geometry>& geometry);

    ///
    /// \brief Get the master geometry
    ///
    virtual std::shared_ptr<Geometry> getMasterGeometry() const override;

    ///
    /// \brief
    ///
    std::shared_ptr<GeometryMap> getCollidingToVisualMap() const;
    void setCollidingToVisualMap(std::shared_ptr<GeometryMap> map);

    ///
    /// \brief Set/Get the force to be applied to the object
    ///
    const Vec3d& getForce() const;
    void setForce(Vec3d force);
    void resetForce();
    void appendForce(Vec3d force);

    ///
    /// \brief Updates the geometries from the maps (if defined)
    ///
    virtual void updateGeometries() override;

    ///
    /// \brief Initialize the scene object
    ///
    virtual bool initialize() override;

protected:
    std::shared_ptr<Geometry>    m_collidingGeometry;    ///> Geometry for collisions
    std::shared_ptr<GeometryMap> m_collidingToVisualMap; ///> Maps transformations to visual geometry
    Vec3d m_force = Vec3d::Zero();
};

using StaticObject = CollidingObject;
}// imstk
