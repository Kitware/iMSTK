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

// imstk
#include "imstkCollisionHandling.h"

#include <vector>

namespace imstk
{
class PbdObject;
class PbdCollisionConstraint;
class PbdEdgeEdgeConstraint;
class PbdPointTriangleConstraint;
class PbdCollisionSolver;
struct CollisionData;

///
/// \class PBDCollisionHandling
///
/// \brief Implements PBD based collision handling
///
class PBDCollisionHandling : public CollisionHandling
{
public:

    ///
    /// \brief Constructor
    ///
    PBDCollisionHandling(const Side&                          side,
                         const std::shared_ptr<CollisionData> colData,
                         std::shared_ptr<PbdObject> pbdObject1,
                         std::shared_ptr<PbdObject> pbdObject2);

    PBDCollisionHandling() = delete;

    ///
    /// \brief Destructor, clear memory pool
    ///
    virtual ~PBDCollisionHandling() override;

    ///
    /// \brief Compute forces based on collision data
    ///
    void processCollisionData() override;

    ///
    /// \brief Generate appropriate PBD constraints based on the collision data
    ///
    void generatePBDConstraints();

    std::shared_ptr<PbdCollisionSolver> getCollisionSolver() const { return m_pbdCollisionSolver; }

private:
    std::shared_ptr<PbdObject> m_PbdObject1 = nullptr; ///> PBD object
    std::shared_ptr<PbdObject> m_PbdObject2 = nullptr; ///> PBD object
    std::shared_ptr<PbdCollisionSolver> m_pbdCollisionSolver = nullptr;

    std::vector<PbdCollisionConstraint*> m_PBDConstraints; ///> List of PBD constraints

    std::vector<PbdEdgeEdgeConstraint*>      m_EEConstraintPool;
    std::vector<PbdPointTriangleConstraint*> m_VTConstraintPool;
};
}
