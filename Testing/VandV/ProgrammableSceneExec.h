/* This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details. */

#pragma once

#include "ProgrammableScene.h"
#include "Verification/CSVUtils.h"

namespace imstk
{
class ProgrammableSceneExec
{
public:
    ProgrammableSceneExec() {};
    ~ProgrammableSceneExec() = default;

    bool executeScene(std::shared_ptr<ProgrammableScene>);
    void report();

    bool m_viewScene;

protected:
    std::vector<std::shared_ptr<ProgrammableScene>> m_executedScenes;
};
} // namespace imstk
