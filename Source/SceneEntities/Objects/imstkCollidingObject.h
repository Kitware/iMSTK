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

///
/// \class CollidingObject
///
/// \brief A SceneObject with a geometry for collision
///
class CollidingObject : public SceneObject
{
public:
    CollidingObject(const std::string& name) : SceneObject(name) { }
    virtual ~CollidingObject() override = default;

    virtual const std::string getTypeName() const override { return "CollidingObject"; }

    ///
    /// \brief
    ///
    std::shared_ptr<Geometry> getCollidingGeometry() const;
    void setCollidingGeometry(std::shared_ptr<Geometry> geometry);

    ///
    /// \brief
    ///
    std::shared_ptr<GeometryMap> getCollidingToVisualMap() const;
    void setCollidingToVisualMap(std::shared_ptr<GeometryMap> map);

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
