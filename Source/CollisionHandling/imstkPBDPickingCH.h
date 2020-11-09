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

#include "imstkCollisionHandling.h"

#include "imstkMath.h"
namespace imstk
{
class CollidingObject;
class PbdObject;
struct CollisionData;

///
/// \class PBDPickingCH
///
/// \brief Implements nodal picking for PBD object
///
class PBDPickingCH : public CollisionHandling
{
public:

    ///
    /// \brief Constructor
    ///
    PBDPickingCH(const Side&                          side,
                 const std::shared_ptr<CollisionData> colData,
                 std::shared_ptr<PbdObject>           pbdObj,
                 std::shared_ptr<CollidingObject>     pickObj);

    PBDPickingCH() = delete;

    ///
    /// \brief Destructor
    ///
    virtual ~PBDPickingCH() override = default;

    ///
    /// \brief Compute forces based on collision data
    ///
    void processCollisionData() override;

    ///
    /// \brief Add picking constraints for the node that is picked
    ///
    void addPickConstraints(std::shared_ptr<PbdObject> pbdObj, std::shared_ptr<CollidingObject> pickObj);

    ///
    /// \brief Update picking constraints for the node that is picked
    ///
    void updatePickConstraints();

    ///
    /// \brief Remove picking constraints for the node that is picked
    ///
    void removePickConstraints();

    ///
    /// \brief Activate picking constraints for nodes in the collision data
    ///
    void activatePickConstraints();

private:
    bool m_isPicking;
    std::map<size_t, Vec3d>          m_pickedPtIdxOffset;    ///> Map for picked nodes.
    std::shared_ptr<PbdObject>       m_pbdObj;               ///> PBD object
    std::shared_ptr<CollidingObject> m_pickObj;              ///> Picking tool object
};
}
