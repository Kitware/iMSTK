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

#ifndef imstkInteractionPair_h
#define imstkInteractionPair_h

#include <map>
#include <memory>

#include "imstkCollidingObject.h"
#include "imstkCollisionDetection.h"
#include "imstkCollisionData.h"
#include "imstkCollisionHandling.h"

namespace imstk {

class InteractionPair
{
    using ObjectsPair = std::pair<std::shared_ptr<CollidingObject>, std::shared_ptr<CollidingObject>>;
    using CollisionDataMap = std::map<std::shared_ptr<CollidingObject>, CollisionData>;
    using CollisionHandlingMap = std::map<std::shared_ptr<CollidingObject>, std::shared_ptr<CollisionHandling>>;

public:

    ///
    /// \brief Constructor
    ///
    InteractionPair(std::shared_ptr<CollidingObject> A,
                    std::shared_ptr<CollidingObject> B,
                    CollisionDetection::Type CDType,
                    CollisionHandling::Type CHAType,
                    CollisionHandling::Type CHBType);

    ///
    /// \brief Destructor
    ///
    ~InteractionPair() = default;

    ///
    /// \brief Call collision detection algorithm to compute collision data
    ///
    void computeCollisionData();

    ///
    /// \brief Call collision handling algorithm to compute contact forces for an object
    ///
    void computeContactForces(std::shared_ptr<CollidingObject> A);

    ///
    /// \brief Call collision handling algorithm to compute contact forces for an object
    ///
    const bool& isValid();

    ///
    /// \brief Returns objects pair
    ///
    const ObjectsPair& getObjectsPair() const;

protected:

    ObjectsPair m_objects;                           //!< Colliding objects
    std::shared_ptr<CollisionDetection> m_colDetect; //!< Collision detection algorithm
    CollisionDataMap m_colDataMap;                   //!< Map of collision data per colliding object
    CollisionHandlingMap m_colHandlingMap;           //!< Map of collision handling algorithm per colliding object

    bool m_valid;
};
}

#endif // ifndef imstkInteractionPair_h
