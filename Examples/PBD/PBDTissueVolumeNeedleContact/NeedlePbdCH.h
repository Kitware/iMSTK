/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMacros.h"
#include "imstkPbdCollisionHandling.h"

#include "NeedleObject.h"

using namespace imstk;

///
/// \brief Surface collision disabled upon puncture
///
class NeedlePbdCH : public PbdCollisionHandling
{
public:
    NeedlePbdCH() = default;
    ~NeedlePbdCH() override = default;

    IMSTK_TYPE_NAME(NeedlePbdCH)

protected:
    ///
    /// \brief Add a vertex-triangle constraint
    ///
    void addVTConstraint(
        VertexMassPair ptA,
        VertexMassPair ptB1, VertexMassPair ptB2, VertexMassPair ptB3,
        double stiffnessA, double stiffnessB) override
    {
        auto needleObj = std::dynamic_pointer_cast<NeedleObject>(getInputObjectB());
        if (needleObj->getCollisionState() == NeedleObject::CollisionState::TOUCHING)
        {
            PbdCollisionHandling::addVTConstraint(ptA, ptB1, ptB2, ptB3, stiffnessA, stiffnessB);
        }
    }

    ///
    /// \brief Add an edge-edge constraint
    ///
    void addEEConstraint(
        VertexMassPair ptA1, VertexMassPair ptA2,
        VertexMassPair ptB1, VertexMassPair ptB2,
        double stiffnessA, double stiffnessB) override
    {
        auto needleObj = std::dynamic_pointer_cast<NeedleObject>(getInputObjectB());
        if (needleObj->getCollisionState() == NeedleObject::CollisionState::TOUCHING)
        {
            PbdCollisionHandling::addEEConstraint(ptA1, ptA2, ptB1, ptB2, stiffnessA, stiffnessB);
        }
    }

    ///
    /// \brief Add a point-edge constraint
    ///
    void addPEConstraint(
        VertexMassPair ptA1,
        VertexMassPair ptB1, VertexMassPair ptB2,
        double stiffnessA, double stiffnessB) override
    {
        auto needleObj = std::dynamic_pointer_cast<NeedleObject>(getInputObjectB());
        if (needleObj->getCollisionState() == NeedleObject::CollisionState::TOUCHING)
        {
            PbdCollisionHandling::addPEConstraint(ptA1, ptB1, ptB2, stiffnessA, stiffnessB);
        }
    }

    ///
    /// \brief Add a point-point constraint
    ///
    void addPPConstraint(
        VertexMassPair ptA, VertexMassPair ptB,
        double stiffnessA, double stiffnessB) override
    {
        auto needleObj = std::dynamic_pointer_cast<NeedleObject>(getInputObjectB());
        if (needleObj->getCollisionState() == NeedleObject::CollisionState::TOUCHING)
        {
            PbdCollisionHandling::addPPConstraint(ptA, ptB, stiffnessA, stiffnessB);
        }
    }
};