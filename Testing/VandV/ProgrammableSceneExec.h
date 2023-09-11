/* This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details. */

#pragma once

#include "ProgrammableScene.h"
#include "Verification/CSVUtils.h"

namespace imstk
{
///
/// \class ProgrammableSceneExec
///
/// \brief Executes a ProgrammableScene to generate data files,
///        Then compares those new files with a baseline file and generates a report with differences
///
class ProgrammableSceneExec
{
public:
    ProgrammableSceneExec() { m_viewScene = false; };
    ~ProgrammableSceneExec() = default;

    bool executeScene(std::shared_ptr<ProgrammableScene>);
    void report();

    bool m_viewScene;

protected:
    std::vector<std::shared_ptr<ProgrammableScene>> m_executedScenes;
};
} // namespace imstk
