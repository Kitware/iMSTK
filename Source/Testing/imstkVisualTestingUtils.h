/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#pragma once

#include <gtest/gtest.h>

namespace imstk
{
class Scene;
class SceneManager;
class SimulationManager;
class VTKViewer;
} // namespace imstk

using namespace imstk;

class VisualTestManager : public testing::Test
{
public:
    void SetUp() override;

    ///
    /// \brief Run the simulation for given duration at given fixed timestep
    /// \param total time (seconds) to run the simulation (-1 for nonstop)
    /// \param fixed timestep (seconds)
    ///
    void runFor(const double duration = -1.0, const double fixedTimestep = 0.001);

protected:
    double m_duration    = 2.0;   ///< Duration to run the test
    double m_dt          = 0.001; ///< Fixed timestep
    bool   m_timerPaused = false;

    std::shared_ptr<VTKViewer>         m_viewer       = nullptr;
    std::shared_ptr<SceneManager>      m_sceneManager = nullptr;
    std::shared_ptr<SimulationManager> m_driver       = nullptr;
    std::shared_ptr<Scene> m_scene = nullptr;
};