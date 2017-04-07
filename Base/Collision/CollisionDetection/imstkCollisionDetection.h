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

#ifndef imstkCollisionDetection_h
#define imstkCollisionDetection_h

#include <memory>

namespace imstk
{

class CollidingObject;
class CollisionData;

///
/// \class CollisionDetection
///
/// \brief Base class for all collision detection classes
///
class CollisionDetection
{
public:

    ///
    /// \brief Type of the collision detection
    ///
    enum class Type
    {
        UnidirectionalPlaneToSphere,
        BidirectionalPlaneToSphere,
        SphereToSphere,
        MeshToSphere,
        MeshToPlane,
        MeshToMesh,
        MeshToCapsule
    };

    ///
    /// \brief Static factory for collision detection sub classes
    ///
    static std::shared_ptr<CollisionDetection> make_collision_detection(const Type& type,
            std::shared_ptr<CollidingObject> objA,
            std::shared_ptr<CollidingObject> objB,
            CollisionData& colData);

    ///
    /// \brief Constructor
    ///
    CollisionDetection(const Type& type, CollisionData& colData) :
        m_type(type),
        m_colData(colData)
    {}
    CollisionDetection() = delete;

    ///
    /// \brief Destructor
    ///
    ~CollisionDetection() = default;

    ///
    /// \brief Detect collision and compute collision data (pure virtual)
    ///
    virtual void computeCollisionData() = 0;

    ///
    /// \brief Returns collision detection type
    ///
    const Type& getType() const;

    ///
    /// \brief Returns collision data
    ///
    const CollisionData& getCollisionData() const;

protected:

    Type m_type;              ///< Collision detection algorithm type
    CollisionData& m_colData; ///< Collision data

};

}

#endif // ifndef imstkCollisionDetection_h
