/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkTextVisualModel.h"

namespace imstk
{
class SceneManager;

///
/// \class SceneControlText
///
/// \brief This implements a text on screen that will notify the user of the current
/// state of the simulation. ie: If paused or not. It can be toggled on and off.
///
class SceneControlText : public TextVisualModel
{
public:
    SceneControlText(const std::string& name = "SceneControlText");

    ///
    /// \brief If on, text status will be shown when paused.
    /// On by default.
    ///@{
    void setUseTextStatus(const bool useTextStatus) { m_useTextStatus = useTextStatus; }
    bool getUseTextStatus() const { return m_useTextStatus; }
    ///@}

    ///
    /// \brief Set the scene manager whose fps we should track
    ///
    void setSceneManager(std::weak_ptr<SceneManager> sceneManager) { m_sceneManager = sceneManager; }

protected:
    void init() override;

protected:
    std::weak_ptr<SceneManager> m_sceneManager;
    bool m_useTextStatus = true;
};
} // namespace imstk