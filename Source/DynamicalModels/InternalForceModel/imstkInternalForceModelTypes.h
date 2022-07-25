/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

namespace imstk
{
///
/// \enum class
///
/// \brief Finite element method type
///
enum class FeMethodType
{
    StVK,
    Corotational,
    Linear,
    Invertible
};

/// \todo Move to appropriate place
enum class HyperElasticMaterialType
{
    StVK,
    NeoHookean,
    MooneyRivlin,
    none
};
} // namespace imstk