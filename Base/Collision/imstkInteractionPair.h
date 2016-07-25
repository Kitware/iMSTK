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

// std library
#include <map>
#include <memory>

// imstk
#include "imstkCollidingObject.h"
#include "imstkCollisionDetection.h"
#include "imstkCollisionData.h"
#include "imstkCollisionHandling.h"

namespace imstk
{

///
/// \class InteractionPair
///
/// \brief This class implements collision interaction between two given scene objects
///
class InteractionPair
{
    using ObjectsPair = std::pair<std::shared_ptr<CollidingObject>, std::shared_ptr<CollidingObject>>;

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
    void computeContactForces();

    ///
    /// \brief Call collision handling algorithm to compute contact forces for an object
    ///
    const bool& isValid();

    ///
    /// \brief Returns objects pair
    ///
    const ObjectsPair& getObjectsPair() const;

protected:

    ObjectsPair m_objects;                             ///< Colliding objects
    std::shared_ptr<CollisionDetection> m_colDetect;   ///< Collision detection algorithm
    CollisionData m_colData;                           ///< Common Collision Data
    std::shared_ptr<CollisionHandling> m_colHandlingA; ///< Collision handling algorithm for A
    std::shared_ptr<CollisionHandling> m_colHandlingB; ///< Collision handling algorithm for B

    bool m_valid;
};

}

#endif // ifndef imstkInteractionPair_h
