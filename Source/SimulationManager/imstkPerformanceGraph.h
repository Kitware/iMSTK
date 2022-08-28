/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkComponent.h"

namespace imstk
{
class SceneManager;
//class TextVisualModel;
class Viewer;

///
/// \class PerformanceGraph
///
/// \brief Displays task graph node durations of a scene as overlay on the screen
/// \todo: There is no delegate for this yet, it hooks directly into VTKRenderer
///
class PerformanceGraph : public Behaviour<double>
{
public:
    PerformanceGraph(const std::string& name = "PerformanceGraph") : Behaviour<double>(name) { }

    ///
    /// \brief Update the display of the last frames update times
    ///
    void visualUpdate(const double& dt);

    void setViewer(std::weak_ptr<Viewer> viewer) { m_viewer = viewer; }
    void setSceneManager(std::weak_ptr<SceneManager> sceneManager) { m_sceneManager = sceneManager; }

protected:
    //std::shared_ptr<TextVisualModel> m_fpsTextVisualModel;
    std::weak_ptr<Viewer>       m_viewer;
    std::weak_ptr<SceneManager> m_sceneManager;
    int    m_prevInfoLevel = -1;
    double m_t = 0.0;
    double m_updateDuration = 0.15; ///< seconds long to wait before updating again
    bool   m_enabled = false;
};
} // namespace imstk