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

#ifndef imstkSceneObject_h
#define imstkSceneObject_h

#include <memory>
#include <string>

#include "imstkGeometry.h"
#include "imstkGeometryMap.h"

namespace imstk
{

//class Geometry;
//class GeometryMap;

///
/// \class SceneObject
///
/// \brief Base class for all scene objects. A scene object can optionally be visible and
/// collide with other scene objects. A object of the class is static.
///
class SceneObject
{
public:
    enum class Type
    {
        Static,
        Dynamic,
        Rigid,
        Deformable,
        VirtualCoupling
    };

    ///
    /// \brief Constructor
    ///
    SceneObject(std::string name) : m_name(name)
    {
        SceneObject::Type::Static;
    }

    ///
    /// \brief Destructor
    ///
    virtual ~SceneObject() = default;

    ///
    /// \brief Get the type of the object
    ///
    const Type& getType() const;

    ///
    /// \brief Get/Set the custom name of the scene object
    ///
    const std::string& getName() const;
    void setName(const std::string& name);

    ///
    /// \brief Get/Set geometry used for viewing
    ///
    std::shared_ptr<Geometry> getVisualGeometry() const;
    void setVisualGeometry(std::shared_ptr<Geometry> geometry);

    ///
    /// \brief Set/Get the geometry used for collisions
    ///
    std::shared_ptr<Geometry> getCollidingGeometry() const;
    void setCollidingGeometry(std::shared_ptr<Geometry> geometry);

    ///
    /// \brief Set/Get the Colliding-to-Visual map
    ///
    std::shared_ptr<GeometryMap> getCollidingToVisualMap() const;
    void setCollidingToVisualMap(std::shared_ptr<GeometryMap> map);

protected:
    ///
    /// \brief Assigns the type of the object
    ///
    void setType(Type type);

    Type m_type = Type::Static; ///> Type of the scene object
    std::string m_name; ///> Custom name of the scene object

    std::shared_ptr<Geometry> m_visualGeometry;          ///> Geometry for rendering
    std::shared_ptr<Geometry> m_collidingGeometry;       ///> Geometry for collisions
    std::shared_ptr<GeometryMap> m_collidingToVisualMap; ///> Maps transformations to visual geometry
};

using VisualObject = SceneObject;

} // imstk

#endif // ifndef imstkSceneObject_h
