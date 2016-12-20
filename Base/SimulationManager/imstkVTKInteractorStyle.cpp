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

// imstk
#include "imstkVTKInteractorStyle.h"
#include "imstkSimulationManager.h"

// vtk
#include "vtkObjectFactory.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkCamera.h"
#include "vtkAssemblyPath.h"
#include "vtkAbstractPropPicker.h"

namespace imstk
{

vtkStandardNewMacro(VTKInteractorStyle);

void
VTKInteractorStyle::OnTimer()
{
    if (m_simManager->getStatus() != SimulationStatus::RUNNING)
    {
        this->Interactor->CreateOneShotTimer(m_targetMS);
        return;
    }

    // Update Camera
    m_simManager->getViewer()->getCurrentRenderer()->updateSceneCamera(m_simManager->getCurrentScene()->getCamera());

    // Update render delegates
    m_simManager->getViewer()->getCurrentRenderer()->updateRenderDelegates();

    // Reset camera clipping range
    this->CurrentRenderer->ResetCameraClippingRange();

    // Retrieve actual framerate
    auto t = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()-m_pre).count();
    auto fps = 1000.0/(double)t;
    auto fpsStr = std::to_string((int)fps)+ " fps";

    // Render
    m_pre = std::chrono::high_resolution_clock::now();
    this->Interactor->Render();
    m_post = std::chrono::high_resolution_clock::now();

    // Plan next render
    auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(m_post-m_pre).count();
    if (dt < m_targetMS)
    {
        this->Interactor->CreateOneShotTimer(m_targetMS - dt);
    }
    else
    {
        this->Interactor->CreateOneShotTimer(0);
    }

    // Timing info
    std::cout << "\rActual framerate: " << fpsStr << " (" << dt << " ms)             "<< std::flush;
}

void
VTKInteractorStyle::OnChar()
{
    char key = this->Interactor->GetKeyCode();

    // Call custom function if exists, and return
    // if it returned `override=true`
    if(m_onCharFunctionMap.count(key) &&
       m_onCharFunctionMap.at(key) &&
       m_onCharFunctionMap.at(key)(this))
    {
      return;
    }

    SimulationStatus status = m_simManager->getStatus();

    // start simulation
    if (status == SimulationStatus::INACTIVE && (key == 's' || key == 'S'))
    {
        m_simManager->startSimulation();
    }
    // end Simulation
    else if (status != SimulationStatus::INACTIVE &&
             (key == 'q' || key == 'Q' || key == 'e' || key == 'E'))
    {
        m_simManager->endSimulation();
    }
    else if (key == ' ')
    {
        // pause simulation
        if (status == SimulationStatus::RUNNING)
        {
            m_simManager->pauseSimulation();
        }
        // play simulation
        else if (status == SimulationStatus::PAUSED)
        {
            m_simManager->runSimulation();
        }
    }
    // quit viewer
    else if (key == '\u001B')
    {
        m_simManager->getViewer()->endRenderingLoop();
    }
}

void
VTKInteractorStyle::OnMouseMove()
{
    // Call custom function if exists, and return
    // if it returned `override=true`
    if(m_onMouseMoveFunction &&
       m_onMouseMoveFunction(this))
    {
      return;
    }

    // Default behavior : ignore mouse if simulation active
    if (m_simManager->getStatus() != SimulationStatus::INACTIVE)
    {
        return;
    }

    // Else : use base class interaction
    vtkBaseInteractorStyle::OnMouseMove();
}

void
VTKInteractorStyle::OnLeftButtonDown()
{
    // Call custom function if exists, and return
    // if it returned `override=true`
    if(m_onLeftButtonDownFunction &&
       m_onLeftButtonDownFunction(this))
    {
      return;
    }

    // Default behavior : ignore mouse if simulation active
    if (m_simManager->getStatus() != SimulationStatus::INACTIVE)
    {
        return;
    }

    // Else : use base class interaction
    vtkBaseInteractorStyle::OnLeftButtonDown();
}

void
VTKInteractorStyle::OnLeftButtonUp()
{
    // Call custom function if exists, and return
    // if it returned `override=true`
    if(m_onLeftButtonUpFunction &&
       m_onLeftButtonUpFunction(this))
    {
      return;
    }

    // Default behavior : ignore mouse if simulation active
    if (m_simManager->getStatus() != SimulationStatus::INACTIVE)
    {
        return;
    }

    // Else : use base class interaction
    vtkBaseInteractorStyle::OnLeftButtonUp();
}

void
VTKInteractorStyle::OnMiddleButtonDown()
{
    // Call custom function if exists, and return
    // if it returned `override=true`
    if(m_onMiddleButtonDownFunction &&
       m_onMiddleButtonDownFunction(this))
    {
      return;
    }

    // Default behavior : ignore mouse if simulation active
    if (m_simManager->getStatus() != SimulationStatus::INACTIVE)
    {
        return;
    }

    // Else : use base class interaction
    vtkBaseInteractorStyle::OnMiddleButtonDown();
}

void
VTKInteractorStyle::OnMiddleButtonUp()
{
    // Call custom function if exists, and return
    // if it returned `override=true`
    if(m_onMiddleButtonUpFunction &&
       m_onMiddleButtonUpFunction(this))
    {
      return;
    }

    // Default behavior : ignore mouse if simulation active
    if (m_simManager->getStatus() != SimulationStatus::INACTIVE)
    {
        return;
    }

    // Else : use base class interaction
    vtkBaseInteractorStyle::OnMiddleButtonUp();
}

void
VTKInteractorStyle::OnRightButtonDown()
{
    // Call custom function if exists, and return
    // if it returned `override=true`
    if(m_onRightButtonDownFunction &&
       m_onRightButtonDownFunction(this))
    {
      return;
    }

    // Default behavior : ignore mouse if simulation active
    if (m_simManager->getStatus() != SimulationStatus::INACTIVE)
    {
        return;
    }

    // Else : use base class interaction
    vtkBaseInteractorStyle::OnRightButtonDown();
}

void
VTKInteractorStyle::OnRightButtonUp()
{
    // Call custom function if exists, and return
    // if it returned `override=true`
    if(m_onRightButtonUpFunction &&
       m_onRightButtonUpFunction(this))
    {
      return;
    }

    // Default behavior : ignore mouse if simulation active
    if (m_simManager->getStatus() != SimulationStatus::INACTIVE)
    {
        return;
    }

    // Else : use base class interaction
    vtkBaseInteractorStyle::OnRightButtonUp();
}

void
VTKInteractorStyle::OnMouseWheelForward()
{
    // Call custom function if exists, and return
    // if it returned `override=true`
    if(m_onMouseWheelForwardFunction &&
       m_onMouseWheelForwardFunction(this))
    {
      return;
    }

    // Default behavior : ignore mouse if simulation active
    if (m_simManager->getStatus() != SimulationStatus::INACTIVE)
    {
        return;
    }

    // Else : use base class interaction
    vtkBaseInteractorStyle::OnMouseWheelForward();
}

void
VTKInteractorStyle::OnMouseWheelBackward()
{
    // Call custom function if exists, and return
    // if it returned `override=true`
    if(m_onMouseWheelBackwardFunction &&
       m_onMouseWheelBackwardFunction(this))
    {
      return;
    }

    // Default behavior : ignore mouse if simulation active
    if (m_simManager->getStatus() != SimulationStatus::INACTIVE)
    {
        return;
    }

    // Else : use base class interaction
    vtkBaseInteractorStyle::OnMouseWheelBackward();
}

} // imstk
