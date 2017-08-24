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

#ifndef imstkCollisionHandling_h
#define imstkCollisionHandling_h

// Std library
#include <memory>

// imstk
#include "imstkCollidingObject.h"
#include "imstkCollisionData.h"

namespace imstk
{
class InteractionPair;

///
/// \class CollisionHandling
///
/// \brief Base class for all collision handling classes
///
class CollisionHandling
{
public:
    ///
    /// \brief Type of the collision handling
    ///
    enum class Type
    {
        None,
        Penalty,
        VirtualCoupling,
        NodalPicking,
        BoneDrilling
    };

    ///
    /// \brief Direction of the collision handling
    ///
    enum class Side
    {
        A,
        B,
        AB
    };

    ///
    /// \brief Static factory for collision handling sub classes
    ///
    static std::shared_ptr<CollisionHandling> make_collision_handling(
        const Type& type,
        const Side& side,
        const CollisionData& colData,
        std::shared_ptr<CollidingObject> objA,
        std::shared_ptr<CollidingObject> objB = nullptr);

    ///
    /// \brief Constructor
    ///
    CollisionHandling(const Type& type,
                      const Side& side,
                      const CollisionData& colData) :
        m_type(type),
        m_side(side),
        m_colData(colData){}
    CollisionHandling() = delete;

    ///
    /// \brief Destructor
    ///
    ~CollisionHandling() = default;

    ///
    /// \brief Compute forces based on collision data (pure virtual)
    ///
    virtual void computeContactForces() = 0;

    ///
    /// \brief Returns collision handling type
    ///
    const Type& getType() const;

protected:

    Type m_type;                    ///< Collision handling algorithm type
    Side m_side;                    ///< Direction of the collisionData
    const CollisionData& m_colData; ///< Collision data
};
}

#endif // ifndef imstkCollisionHandling_h