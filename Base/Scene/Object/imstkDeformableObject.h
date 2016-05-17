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

#ifndef imstkDeformableObject_h
#define imstkDeformableObject_h

#include <memory>

#include "imstkCollidingObject.h"

namespace imstk {

class Geometry;
class GeometryMap;

class DeformableObject : public CollidingObject
{
public:

    DeformableObject(std::string name) : CollidingObject(name)
    {
        m_type = Type::Deformable;
    }

    ~DeformableObject() = default;

    std::shared_ptr<Geometry> getPhysicsGeometry() const;
    void setPhysicsGeometry(std::shared_ptr<Geometry> geometry);

protected:

    std::shared_ptr<Geometry> m_physicsGeometry;          ///> Geometry for collisions
    std::shared_ptr<GeometryMap> m_physicsToCollidingMap; ///> Maps transformations to colliding geometry

};

}

#endif // ifndef imstkDeformableObject_h
