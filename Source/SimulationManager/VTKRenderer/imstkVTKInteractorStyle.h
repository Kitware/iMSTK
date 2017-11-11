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

#ifndef imstkVTKInteractorStyle_h
#define imstkVTKInteractorStyle_h

#include <memory>
#include <chrono>
#include <unordered_map>
#include <functional>

#include "imstkInteractorStyle.h"

#include "vtkInteractorStyleTrackballCamera.h"
class vtkTextActor;

namespace imstk
{
class SimulationManager;

/// Base class of the vtk interactor style used
using vtkBaseInteractorStyle = vtkInteractorStyleTrackballCamera;

///
/// \class classname
///
/// \brief
///
class VTKInteractorStyle : public vtkBaseInteractorStyle, public InteractorStyle
{
public:
    vtkTypeMacro(VTKInteractorStyle, vtkBaseInteractorStyle);

    VTKInteractorStyle();
    virtual ~VTKInteractorStyle();

    ///
    /// \brief Set current renderer
    ///
    virtual void SetCurrentRenderer(vtkRenderer* ren) override;

    ///
    /// \brief Slot for timer tick
    ///
    virtual void OnTimer() override;

    ///
    /// \brief Slot for key pressed
    ///
    virtual void OnChar() override;

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
    virtual void OnFourthButtonDown(){};
    virtual void OnFifthButtonDown(){};
    virtual void OnFourthButtonUp(){};
    virtual void OnFifthButtonUp(){};

private:
    friend class VTKViewer;

    SimulationManager* m_simManager; ///> SimulationManager owning the current simulation being interacted with
    double m_targetMS; ///> expected time between each render frame (in ms)
    std::chrono::high_resolution_clock::time_point m_pre; ///> time point pre-rendering
    std::chrono::high_resolution_clock::time_point m_post; ///> time point post-rendering
    bool m_displayFps; ///> hide or display framerate
    vtkTextActor* m_fpsActor; ///> text holding framerate to display
    std::chrono::high_resolution_clock::time_point m_lastFpsUpdate; ///> time point for last framerate display update
    double m_lastFps; ///> last framerate value used for moving average estimate
};
} // imstk

#endif // ifndef imstkInteractorStyle_h
