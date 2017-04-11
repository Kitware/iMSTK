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

#ifndef imstkPbdInteractionPair_h
#define imstkPbdInteractionPair_h

#include "imstkInteractionPair.h"
#include "imstkPbdEdgeEdgeCollisionConstraint.h"
#include "imstkPbdPointTriCollisionConstraint.h"
#include "imstkPbdObject.h"

namespace imstk
{

///
/// \class PbdInteractionPair
///
/// \brief
///
class PbdInteractionPair
{
public:
    ///
    /// \brief Constructor
    ///
    PbdInteractionPair(std::shared_ptr<PbdObject> A, std::shared_ptr<PbdObject> B):
        first(A), second(B) {}

    ///
    /// \brief Clear the collisions from previous step
    ///
    inline void resetConstraints()
    {
        m_collisionConstraints.clear();
    }

    ///
    /// \brief
    ///
    inline void setNumberOfInterations(const unsigned int& n)
    {
        maxIter = n;
    }

    ///
    /// \brief Broad phase collision detection using AABB
    ///
    bool doBroadPhaseCollision();

    ///
    /// \brief Narrow phase collision detection
    ///
    void doNarrowPhaseCollision();

    ///
    /// \brief Resolves the collision by solving pbd collision constraints
    ///
    void resolveCollision();

private:
    std::vector<std::shared_ptr<PbdCollisionConstraint>> m_collisionConstraints;
    std::shared_ptr<PbdObject> first;
    std::shared_ptr<PbdObject> second;
    unsigned int maxIter;
};

}

#endif // imstkPbdInteractionPair_h
