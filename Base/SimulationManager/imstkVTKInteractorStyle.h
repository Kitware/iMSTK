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

#include "vtkInteractorStyleTrackballCamera.h"

namespace imstk
{

class SimulationManager;

using vtkBaseInteractorStyle = vtkInteractorStyleTrackballCamera;

///
/// \class classname
///
/// \brief
///
class VTKInteractorStyle : public vtkBaseInteractorStyle
{
public:

    static VTKInteractorStyle *New();
    vtkTypeMacro(VTKInteractorStyle, vtkBaseInteractorStyle);

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

protected:

    friend class VTKViewer;

    ///
    /// \brief Define SimulationManager that owns this viewer/interactorStyle
    /// to be able to control the simulation through user interaction
    ///
    void setSimulationManager(SimulationManager* simManager);

    ///
    /// \brief Get the target FPS for rendering
    ///
    double getTargetFrameRate() const;

    ///
    /// \brief Set the target FPS for rendering
    ///
    void setTargetFrameRate(const double& fps);

private:

    SimulationManager* m_simManager; ///> SimulationManager owning the current simulation being interacted with
    double m_targetMS = 0.0; ///> expected time between each render frame (in ms)
    std::chrono::high_resolution_clock::time_point m_pre; ///> time point pre-rendering
    std::chrono::high_resolution_clock::time_point m_post; ///> time point post-rendering

};

} // imstk

#endif // ifndef imstkInteractorStyle_h
