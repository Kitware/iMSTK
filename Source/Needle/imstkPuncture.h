/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMath.h"

#include <unordered_map>

namespace imstk
{
///
/// \brief Punctures are identified via three ints.
/// The needle id, the puncturable id, and a local id that allows multi punctures
/// on the needle,puncture pair. This could be a cell (face) id, vertex id, or
/// an index into some other structure.
///
using PunctureId = std::tuple<int, int, int>;

///
/// \brief The puncture itself is composed of a state and extra
/// non-essential user data.
///
struct Puncture
{
    public:
        enum class State
        {
            REMOVED,
            TOUCHING,
            INSERTED
        };
        ///
        /// \brief Accompanying data stored with a puncture. No usage within Needle
        /// structure.
        ///
        struct UserData
        {
            int id = -1;
            int ids[4];
            Vec4d weights = Vec4d::Zero();
        };

    public:
        //PunctureId id;
        State state = State::REMOVED;
        UserData userData;
};

///
/// \struct PunctureIdHash
///
/// \brief The entity and tissue id should be reversible
///
struct PunctureIdHash
{
    std::size_t operator()(const imstk::PunctureId& k) const
    {
        // Swapping 0 and 1 should result in equivalence.
        return cantor(symCantor(std::get<0>(k), std::get<1>(k)), std::get<2>(k));
    }
};
///
/// \struct PunctureIdEq
///
/// \brief The entity and tissue id should be reversible
///
struct PunctureIdEq
{
    bool operator()(const imstk::PunctureId& a, const imstk::PunctureId& b) const
    {
        std::pair<int, int> a1 = { std::get<0>(a), std::get<1>(a) };
        if (a1.second < a1.first)
        {
            std::swap(a1.first, a1.second);
        }
        std::pair<int, int> b1 = { std::get<0>(b), std::get<1>(b) };
        if (b1.second < b1.first)
        {
            std::swap(b1.first, b1.second);
        }
        return (a1.first == b1.first) && (a1.second == b1.second) && (std::get<2>(a) == std::get<2>(b));
    }
};

using PunctureMap = std::unordered_map<PunctureId, std::shared_ptr<Puncture>, PunctureIdHash, PunctureIdEq>;
} // namespace imstk