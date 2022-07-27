/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMouseControl.h"

namespace imstk
{
class MouseDeviceClient;
class SceneManager;

///
/// \class MouseSceneControl
///
/// \brief Controls the camera using trackball controls
/// Left click rotate, middle click pan
/// \todo: Make base class to control just a scene, subclass for debug toggle
/// or remove debug completely
///
class MouseSceneControl : public MouseControl
{
public:
    enum class Mode
    {
        None,
        Pan,
        Rotate
    };

    MouseSceneControl(const std::string& name = "MouseSceneControl") : MouseControl(name) { }
    ~MouseSceneControl() override = default;

    ///
    /// \brief Set the scroll controlled zoom speed
    ///
    void setZoomSpeed(const double zoomSpeed) { m_zoomSpeed = zoomSpeed; }

    ///
    /// \brief Set the mouse controlled rotate speed
    ///
    void setRotateSpeed(const double rotateSpeed) { m_rotateSpeed = rotateSpeed; }

    ///
    /// \brief Set the pan speed
    ///
    void setPanSpeed(const double panSpeed) { m_panSpeed = panSpeed; }

    ///
    /// \brief Set the scene manager to be controlled
    /// The active scene's camera will be controllable depending on SceneManager's mode, or the
    ///
    void setSceneManager(std::shared_ptr<SceneManager> manager) { m_sceneManager = manager; }

    ///
    /// \brief Enable the mouse control, independent of the debug mode
    ///
    void setEnabled(bool enable);

    ///
    /// \return true if the controls are enabled, either explicitly or debug is on in the scenecontrol
    ///
    bool getEnabled() const;

    void printControls() override;

    ///
    /// \brief On the mouse scene control button press
    ///
    void OnButtonPress(const int key) override;
    void OnButtonRelease(const int key) override;
    void OnScroll(const double dx) override;
    void OnMouseMove(const Vec2d& pos) override;

protected:
    std::shared_ptr<SceneManager> m_sceneManager;
    Mode  m_mode = Mode::None;
    Vec2d m_prevPos;
    Vec2d m_pos;

    // User changeable values
    double m_zoomSpeed   = 1.0;
    double m_rotateSpeed = 1.0;
    double m_panSpeed    = 1.0;

    // Hardcoded values
    double m_zoomFactor   = 1.0;
    double m_rotateFactor = 5.0;
    double m_panFactor    = 1.0;

    bool m_enabled = false;
};
} // namespace imstk