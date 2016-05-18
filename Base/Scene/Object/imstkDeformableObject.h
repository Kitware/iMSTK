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
#include <string>

#include "imstkCollidingObject.h"

namespace imstk {

class Geometry;
class GeometryMap;

///
/// \class DeformableObject
///
/// \brief Base class for scene objects that deform
///
class DeformableObject : public CollidingObject
{
public:

    ///
    /// \brief Constructor
    ///
    DeformableObject(std::string name) : CollidingObject(name)
    {
        m_type = Type::Deformable;
    }

    ///
    /// \brief Destructor
    ///
    ~DeformableObject() = default;

    ///
    /// \brief Returns the geometry used for Physics computations
    ///
    std::shared_ptr<Geometry> getPhysicsGeometry() const;

    ///
    /// \brief Assigns the geometry used for Physics related computations
    ///
    void setPhysicsGeometry(std::shared_ptr<Geometry> geometry);

    ///
    /// \brief Returns the number of degree of freedom
    ///
    int getNumOfDOF() const;

protected:

    std::shared_ptr<Geometry> m_physicsGeometry;                ///> Geometry used for Physics
    std::shared_ptr<GeometryMap> m_physicsToCollidingGeomMap;   ///> Maps transformations to colliding geometry
    std::shared_ptr<GeometryMap> m_physicsToVisualGeomMap;      ///> Maps transformations to colliding geometry

    int numDOF; ///> Number of degree of freedom of the body in the discretized model
};

}

#endif // ifndef imstkDeformableObject_h
