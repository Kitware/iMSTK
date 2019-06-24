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

#pragma once

// std library
#include <memory>

// imstk
#include "imstkCollisionHandling.h"

namespace imstk
{
class CollidingObject;
class CollisionData;
class PbdObject;
class PbdCollisionConstraint;
class PbdSolver;

///
/// \class PBDCollisionHandling
///
/// \brief Implements PBD based collision handling
///
class PBDCollisionHandling : public CollisionHandling
{
typedef std::vector<std::shared_ptr<PbdCollisionConstraint>> PBDConstraintVector;
public:

    ///
    /// \brief Constructor
    ///
    PBDCollisionHandling(const Side& side,
                         const std::shared_ptr<CollisionData> colData,
                         std::shared_ptr<PbdObject> obj1,
                         std::shared_ptr<PbdObject> obj2,
                         std::shared_ptr<PbdSolver> PBDSolver = nullptr) :
        CollisionHandling(Type::Penalty, side, colData),
        m_pbdObject1(obj1),
        m_pbdObject2(obj2),
        m_PBDSolver(PBDSolver){}

    PBDCollisionHandling() = delete;

    ///
    /// \brief Destructor
    ///
    ~PBDCollisionHandling() = default;

    ///
    /// \brief Compute forces based on collision data
    ///
    void processCollisionData() override;

    ///
    /// \brief Generate appropriate PBD constraints based on the collision data
    ///
    void generatePBDConstraints();

private:

    std::shared_ptr<PbdObject> m_pbdObject1;   ///> PBD object
    std::shared_ptr<PbdObject> m_pbdObject2;   ///> PBD object
    PBDConstraintVector m_PBDConstraints;      ///> List of PBD constraints
    std::shared_ptr<PbdSolver> m_PBDSolver;        /// The Solver for the collision constraints
};
}

