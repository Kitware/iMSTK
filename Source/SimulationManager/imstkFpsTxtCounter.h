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
class TextVisualModel;
class Viewer;

///
/// \class FpsTxtCounter
///
/// \brief Displays visual and physics framerates through dependent
/// TextVisualModel
///
class FpsTxtCounter : public Behaviour<double>
{
public:
    FpsTxtCounter(const std::string& name = "FpsTxtCounter") : Behaviour<double>(name) { }

    ///
    /// \brief Update the display of the last frames update times
    ///
    void visualUpdate();

    void setViewer(std::weak_ptr<Viewer> viewer) { m_viewer = viewer; }
    void setSceneManager(std::weak_ptr<SceneManager> sceneManager) { m_sceneManager = sceneManager; }

protected:
    void init() override;

protected:
    std::shared_ptr<TextVisualModel> m_fpsTextVisualModel;
    std::weak_ptr<Viewer>       m_viewer;
    std::weak_ptr<SceneManager> m_sceneManager;
    int m_prevInfoLevel = -1;
};
} // namespace imstk