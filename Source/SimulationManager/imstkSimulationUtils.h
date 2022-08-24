/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include <memory>

namespace imstk
{
class Entity;
class SimulationManager;

namespace SimulationUtils
{
///
/// \brief Create default components for the scene. These aren't applicable
/// to all scene's but commonly used
///
std::shared_ptr<Entity> createDefaultSceneControlEntity(
    std::shared_ptr<SimulationManager> driver);
} // namespace SimulationUtils
} // namespace imstk