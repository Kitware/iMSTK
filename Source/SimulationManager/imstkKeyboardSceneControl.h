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
class SceneControlText;
class SceneManager;

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
    KeyboardSceneControl(const std::string& name = "KeyboardSceneControl") : KeyboardControl(name) { }
    ~KeyboardSceneControl() override = default;

    ///
    /// \brief The driver is used to stop the simulation
    ///
    void setModuleDriver(std::weak_ptr<ModuleDriver> driver) { m_driver = driver; }

    ///
    /// \brief Set the scene manager whose fps we should track
    ///
    void setSceneManager(std::weak_ptr<SceneManager> sceneManager) { m_sceneManager = sceneManager; }

    void setSceneControlText(std::shared_ptr<SceneControlText> sceneControlText) { m_sceneControlText = sceneControlText; }

    void printControls() override;

    void OnKeyPress(const char key) override;

protected:
    std::weak_ptr<ModuleDriver>       m_driver;
    std::weak_ptr<SceneManager>       m_sceneManager;
    std::shared_ptr<SceneControlText> m_sceneControlText;
};
} // namespace imstk