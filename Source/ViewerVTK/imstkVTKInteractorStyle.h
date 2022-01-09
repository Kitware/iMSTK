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
    virtual ~VTKInteractorStyle() override;

    ///
    /// \brief Slot for key pressed
    ///
    virtual void OnKeyPress() override;

    ///
    /// \brief Slot for key released
    ///
    virtual void OnKeyRelease() override;

    ///
    /// \brief Filter out these events
    ///
    virtual void OnKeyDown() override { }
    virtual void OnKeyUp() override { }
    virtual void OnChar() override { }

    ///
    /// \brief Slot for moved mouse cursor
    ///
    virtual void OnMouseMove() override;

    ///
    /// \brief Slot for mouse left button clicked
    ///
    virtual void OnLeftButtonDown() override;

    ///
    /// \brief Slot for mouse left button released
    ///
    virtual void OnLeftButtonUp() override;

    ///
    /// \brief Slot for mouse middle button clicked
    ///
    virtual void OnMiddleButtonDown() override;

    ///
    /// \brief Slot for mouse middle button released
    ///
    virtual void OnMiddleButtonUp() override;

    ///
    /// \brief Slot for mouse right button clicked
    ///
    virtual void OnRightButtonDown() override;

    ///
    /// \brief Slot for mouse right button released
    ///
    virtual void OnRightButtonUp() override;

    ///
    /// \brief Slot for mouse wheel rolled forward
    ///
    virtual void OnMouseWheelForward() override;

    ///
    /// \brief Slot for mouse wheel rolled backward
    ///
    virtual void OnMouseWheelBackward() override;

    ///
    /// \brief Not implemented
    ///
    virtual void OnFourthButtonDown() override {}
    virtual void OnFifthButtonDown() override {}
    virtual void OnFourthButtonUp() override {}
    virtual void OnFifthButtonUp() override {}

    std::shared_ptr<KeyboardDeviceClient> getKeyboardDeviceClient() const { return m_keyboardDeviceClient; }
    std::shared_ptr<MouseDeviceClient> getMouseDeviceClient() const { return m_mouseDeviceClient; }

private:
    std::shared_ptr<KeyboardDeviceClient> m_keyboardDeviceClient;
    std::shared_ptr<MouseDeviceClient>    m_mouseDeviceClient;
};
} // imstk
