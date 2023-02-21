/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include <gtest/gtest.h>

namespace imstk
{
class PbdMethod;
class Scene;
class SceneManager;
class SimulationManager;
class VTKViewer;
} // namespace imstk

class VisualTest : public testing::Test
{
public:
    void SetUp() override;

    void TearDown() override;

    ///
    /// \brief Run the simulation for given duration at given fixed timestep
    /// \param total time (seconds) to run the simulation (-1 for nonstop)
    /// \param fixed timestep (seconds)
    ///
    void runFor(const double duration = -1.0, const double fixedTimestep = 0.001);

protected:
    double m_duration    = 2.0;   ///< Duration to run the test
    double m_dt          = 0.001; ///< Fixed timestep
    bool   m_timerPaused = false; ///< Pauses the test timer
    bool   m_useStdOut   = false; ///< Enables std out in the logger, default off for testing

    std::shared_ptr<imstk::VTKViewer>         m_viewer;
    std::shared_ptr<imstk::SceneManager>      m_sceneManager;
    std::shared_ptr<imstk::SimulationManager> m_driver;
    std::shared_ptr<imstk::Scene> m_scene;
};
