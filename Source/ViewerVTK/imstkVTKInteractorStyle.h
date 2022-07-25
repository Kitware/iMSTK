/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include <vtkInteractorStyleTrackballCamera.h>
#include <memory>

namespace imstk
{
class KeyboardDeviceClient;
class MouseDeviceClient;

///
/// \class VTKInteractorStyle
///
/// \brief Interactor styles forward their controls to imstk objects
///
class VTKInteractorStyle : public vtkInteractorStyleTrackballCamera
{
public:
    static VTKInteractorStyle* New();
    vtkTypeMacro(VTKInteractorStyle, vtkInteractorStyleTrackballCamera);

    VTKInteractorStyle();
    ~VTKInteractorStyle() override;

    ///
    /// \brief Slot for key pressed
    ///
    void OnKeyPress() override;

    ///
    /// \brief Slot for key released
    ///
    void OnKeyRelease() override;

    ///
    /// \brief Filter out these events
    ///@{
    void OnKeyDown() override { }
    void OnKeyUp() override { }
    void OnChar() override { }
    ///@}

    ///
    /// \brief Slot for moved mouse cursor
    ///
    void OnMouseMove() override;

    ///
    /// \brief Slot for mouse left button clicked
    ///
    void OnLeftButtonDown() override;

    ///
    /// \brief Slot for mouse left button released
    ///
    void OnLeftButtonUp() override;

    ///
    /// \brief Slot for mouse middle button clicked
    ///
    void OnMiddleButtonDown() override;

    ///
    /// \brief Slot for mouse middle button released
    ///
    void OnMiddleButtonUp() override;

    ///
    /// \brief Slot for mouse right button clicked
    ///
    void OnRightButtonDown() override;

    ///
    /// \brief Slot for mouse right button released
    ///
    void OnRightButtonUp() override;

    ///
    /// \brief Slot for mouse wheel rolled forward
    ///
    void OnMouseWheelForward() override;

    ///
    /// \brief Slot for mouse wheel rolled backward
    ///
    void OnMouseWheelBackward() override;

    ///
    /// \brief Not implemented
    ///@{
    void OnFourthButtonDown() override {}
    void OnFifthButtonDown() override {}
    void OnFourthButtonUp() override {}
    void OnFifthButtonUp() override {}
    ///@}

    std::shared_ptr<KeyboardDeviceClient> getKeyboardDeviceClient() const { return m_keyboardDeviceClient; }
    std::shared_ptr<MouseDeviceClient> getMouseDeviceClient() const { return m_mouseDeviceClient; }

private:
    std::shared_ptr<KeyboardDeviceClient> m_keyboardDeviceClient;
    std::shared_ptr<MouseDeviceClient>    m_mouseDeviceClient;
};
} // namespace imstk