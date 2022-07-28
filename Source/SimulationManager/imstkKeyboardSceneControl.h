/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkKeyboardControl.h"

namespace imstk
{
class ModuleDriver;
class SceneManager;
class TextVisualModel;

///
/// \class KeyboardSceneControl
///
/// \brief This implements keyboard controls to control a scene manager and viewer
/// it may operate with both sceneManager and viewer, or only one or the other.
/// warning: This control may pause a thread. Thus it is a bad idea to process it
/// on the thread you're pausing, as you then could not resume.
/// \todo: Thread affinity
///
class KeyboardSceneControl : public KeyboardControl
{
public:

    KeyboardSceneControl(const std::string& name = "KeyboardSceneControl");
    ~KeyboardSceneControl() override = default;

    bool initialize() override;

    ///
    /// \brief The driver is used to stop the simulation
    ///
    void setModuleDriver(std::weak_ptr<ModuleDriver> driver) { m_driver = driver; }

    ///
    /// \brief Set the scene manager whose fps we should track
    ///
    void setSceneManager(std::weak_ptr<SceneManager> sceneManager) { m_sceneManager = sceneManager; }

    ///
    /// \brief If on, text status will be shown when paused.
    /// On by default.
    ///@{
    void setUseTextStatus(const bool useTextStatus) { m_useTextStatus = useTextStatus; }
    bool getUseTextStatus() const { return m_useTextStatus; }
    ///@}

    void printControls() override;

    void OnKeyPress(const char key) override;

    void OnKeyRelease(const char key) override;

protected:
    std::weak_ptr<ModuleDriver>      m_driver;
    std::weak_ptr<SceneManager>      m_sceneManager;
    std::shared_ptr<TextVisualModel> m_textVisualModel;
    bool m_useTextStatus = true;
};
} // namespace imstk