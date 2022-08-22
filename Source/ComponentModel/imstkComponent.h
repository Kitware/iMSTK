/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include <string>

namespace imstk
{
///
/// \class Component
///
/// \brief Represents a part of a component, involved in a system
///
class Component
{
protected:
    Component(const std::string& name = "Component") : m_name(name) { }

public:
    virtual ~Component() = default;

    // \todo: Figure out how to remove
    virtual bool initialize() { return true; }

protected:
    std::string m_name;
};

///
/// \class Behaviour
///
/// \brief A Behaviour represents a single component system
///
template<typename UpdateInfo>
class Behaviour : public Component
{
protected:
    Behaviour(const std::string& name = "Behaviour") : Component(name) { }

public:
    ~Behaviour() override = default;

    virtual void update(const UpdateInfo& updateData) = 0;
};
} // namespace imstk