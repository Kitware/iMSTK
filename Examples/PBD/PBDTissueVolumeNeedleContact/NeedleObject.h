/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkPbdObject.h"

using namespace imstk;

class NeedleObject : public PbdObject
{
public:
    enum class CollisionState
    {
        REMOVED,
        TOUCHING,
        INSERTED
    };

public:
    NeedleObject(const std::string& name);
    ~NeedleObject() override = default;

    IMSTK_TYPE_NAME(NeedleObject)

public:
    void setCollisionState(std::shared_ptr<SceneObject> obj, const CollisionState state) { m_collisionStates[obj] = state; }
    CollisionState getCollisionState(std::shared_ptr<SceneObject> obj) { return m_collisionStates[obj]; }

    ///
    /// \brief Returns the current axes of the needle (tip-tail)
    ///
    const Vec3d getNeedleAxes() const;

protected:
    /// State of collision with given SceneObject
    std::unordered_map<std::shared_ptr<SceneObject>, CollisionState> m_collisionStates;
};