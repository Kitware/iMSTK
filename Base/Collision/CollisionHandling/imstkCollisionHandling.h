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

#include <memory>

#include "imstkCollidingObject.h"
#include "imstkCollisionData.h"
#include "imstkContactForces.h"

namespace imstk {

class InteractionPair;

class CollisionHandling
{
public:

    enum class Type
    {
        None,
        LCP,
        LPC,
        Penalty
    };

    ///
    /// \brief Static factory for collision handling sub classes
    ///
    static std::shared_ptr<CollisionHandling> make_collision_handling(
            const Type& type,
            std::shared_ptr<CollidingObject> obj);

    ///
    /// \brief Constructor
    ///
    CollisionHandling(const Type& type) : m_type(type) {}

    ///
    /// \brief Destructor
    ///
    ~CollisionHandling() = default;

    ///
    /// \brief Compute forces based on collision data (pure virtual)
    ///
    virtual void computeContactForces(std::shared_ptr<CollidingObject> obj,
                                      CollisionData& colData,
                                      ContactForces& contactForces) = 0;

    ///
    /// \brief Returns collision handling type
    ///
    const Type& getType() const;

protected:

    Type m_type; //!< Collision handling algorithm type

};
}

#endif // ifndef imstkCollisionHandling_h
