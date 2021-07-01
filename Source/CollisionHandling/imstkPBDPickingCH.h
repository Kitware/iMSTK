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
class PbdPointPointConstraint;
class PbdObject;
class CollisionData;

///
/// \class PBDPickingCH
///
/// \brief Implements nodal picking for PBD object via PointPointCollisionConstraints
/// All points inside the pickObj are constrained with PointPointConstraints constraining
/// them to their relative position when they were picked
///
class PBDPickingCH : public CollisionHandling
{
public:
    PBDPickingCH();
    ~PBDPickingCH() override = default;

    virtual const std::string getTypeName() const override { return "PBDPickingCH"; }

public:
    ///
    /// \brief Add picking constraints for the node that is picked
    ///
    void addPickConstraints(const CDElementVector<CollisionElement>& elements,
                            std::shared_ptr<PbdObject> pbdObj, std::shared_ptr<CollidingObject> pickObj);

    ///
    /// \brief Remove all picking nodes and constraints
    ///
    void endPick() { m_isPicking = false; }

    ///
    /// \brief Add picking nodes nodes and constraints
    ///
    void beginPick() { m_isPicking = true; }

    ///
    /// \brief Generate pbd constraints for tool-mesh collision
    ///
    void generatePBDConstraints(const CDElementVector<CollisionElement>& elements);

protected:
    ///
    /// \brief Add collision constraints based off contact data
    ///
    void handle(
        const CDElementVector<CollisionElement>& elementsA,
        const CDElementVector<CollisionElement>& elementsB) override;

private:
    bool m_isPicking;
    bool m_isPrevPicking;

    std::map<size_t, Vec3d> m_pickedPtIdxOffset;    ///> Map for picked nodes.

    std::list<Vec3d> constraintPts;
    std::list<Vec3d> constraintVels;

    std::vector<std::shared_ptr<PbdPointPointConstraint>> m_constraints; ///> List of PBD constraints
};
}
