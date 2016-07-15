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

#ifndef imstkCollidingObject_h
#define imstkCollidingObject_h

#include <memory>

#include "imstkSceneObject.h"

namespace imstk {

class Geometry;
class GeometryMap;

class CollidingObject : public SceneObject
{
public:
    ///
    /// \brief
    ///
    CollidingObject(std::string name) : SceneObject(name)
    {
        m_type = SceneObject::Type::Static;
    }

    ///
    /// \brief
    ///
    ~CollidingObject() = default;

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

protected:

    std::shared_ptr<Geometry> m_collidingGeometry;       ///> Geometry for collisions
    std::shared_ptr<GeometryMap> m_collidingToVisualMap; ///> Maps transformations to visual geometry

};

using StaticObject = CollidingObject;
}

#endif // ifndef imstkCollidingObject_h
