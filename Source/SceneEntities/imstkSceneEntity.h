/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkEventObject.h"

#include <atomic>

namespace imstk
{
using EntityID = unsigned long;

///
/// \class SceneEntity
///
/// \brief Top-level class for iMSTK scene elements (scene objects, lights, camera)
///
class SceneEntity : public EventObject
{
public:
    ~SceneEntity() override = default;

    ///
    /// \brief Get ID (ALWAYS query the ID in your code, DO NOT hardcode it)
    /// \returns ID of entity
    ///
    EntityID getID() const;

    virtual const std::string getTypeName() const = 0;

protected:
    SceneEntity();

    // Not the best design pattern
    static std::atomic<EntityID> s_count; ///< current count of entities

    EntityID m_ID;                        ///< unique ID of entity
};
} // namespace imstk
