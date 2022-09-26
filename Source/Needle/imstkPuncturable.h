/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkComponent.h"
#include "imstkPuncture.h"

namespace imstk
{
class Entity;
class Needle;
class Puncturable;

///
/// \class Puncturable
///
/// \brief Place this on an object to make it puncturable by a needle. This allows
/// puncturables to know they've been punctured without needing to be aware of the
/// needle.
/// The Puncturable supports multiple local ids
///
class Puncturable : public Component
{
public:
    Puncturable(const std::string& name = "Puncturable") : Component(name) { }
    ~Puncturable() override = default;

    ///
    /// \brief Get/Set puncture data
    /// @{
    void setPuncture(const PunctureId& id, std::shared_ptr<Puncture> data);
    std::shared_ptr<Puncture> getPuncture(const PunctureId& id);
    /// @}

    bool getPunctured() const;

protected:
    PunctureMap m_punctures;
};

///
/// \brief Get puncture id between needle and puncturable
///
PunctureId getPunctureId(std::shared_ptr<Needle>      needle,
                         std::shared_ptr<Puncturable> puncturable,
                         const int                    supportId = -1);
} // namespace imstk