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
    vtkRenderWindowInteractor *rwi = this->Interactor;

    switch (rwi->GetKeyCode())
    {
    // Highlight picked actor
    case 'p' :
    case 'P' :
    {
        if (m_simManager->getStatus() != SimulationStatus::INACTIVE)
        {
            return;
        }

        if(this->CurrentRenderer != 0)
        {
            if (this->State == VTKIS_NONE)
            {
                vtkAssemblyPath *path = nullptr;
                int *eventPos = rwi->GetEventPosition();
                this->FindPokedRenderer(eventPos[0], eventPos[1]);
                rwi->StartPickCallback();
                auto picker = vtkAbstractPropPicker::SafeDownCast(rwi->GetPicker());
                if ( picker != nullptr )
                {
                    picker->Pick(eventPos[0], eventPos[1], 0.0, this->CurrentRenderer);
                    path = picker->GetPath();
                }
                if ( path == nullptr )
                {
                    this->HighlightProp(nullptr);
                    this->PropPicked = 0;
                }
                else
                {
                    this->HighlightProp(path->GetFirstNode()->GetViewProp());
                    this->PropPicked = 1;
                }
                rwi->EndPickCallback();
            }
        }
        else
        {
            vtkWarningMacro(<<"no current renderer on the interactor style.");
        }
    }
    break;

    // Fly To picked actor
    case 'f' :
    case 'F' :
    {
        if (m_simManager->getStatus() != SimulationStatus::INACTIVE)
        {
            return;
        }

        if(this->CurrentRenderer != 0)
        {
            this->AnimState = VTKIS_ANIM_ON;
            vtkAssemblyPath *path = nullptr;
            int *eventPos = rwi->GetEventPosition();
            this->FindPokedRenderer(eventPos[0], eventPos[1]);
            rwi->StartPickCallback();
            auto picker = vtkAbstractPropPicker::SafeDownCast(rwi->GetPicker());
            if ( picker != nullptr )
            {
                picker->Pick(eventPos[0], eventPos[1], 0.0, this->CurrentRenderer);
                path = picker->GetPath();
            }
            if (path != nullptr)
            {
                rwi->FlyTo(this->CurrentRenderer, picker->GetPickPosition());
            }
            this->AnimState = VTKIS_ANIM_OFF;
        }
        else
        {
            vtkWarningMacro(<<"no current renderer on the interactor style.");
        }
    }
    break;

    // Reset Camera
    case 'r' :
    case 'R' :
    {
        if (m_simManager->getStatus() != SimulationStatus::INACTIVE)
        {
            return;
        }

        if(this->CurrentRenderer!=0)
        {
            this->CurrentRenderer->ResetCamera();
            this->CurrentRenderer->GetActiveCamera()->SetFocalPoint(0,0,0);
        }
        else
        {
            vtkWarningMacro(<<"no current renderer on the interactor style.");
        }
        rwi->Render();
    }
    break;

    // Stop Simulation
    case 's' :
    case 'S' :
    {
        m_simManager->startSimulation();
    }
    break;

    // End Simulation
    case 'q' :
    case 'Q' :
    case 'e' :
    case 'E' :
    {
        m_simManager->endSimulation();
    }
    break;

    // Play/Pause Simulation
    case ' ' :
    {
        if (m_simManager->getStatus() == SimulationStatus::RUNNING)
        {
            m_simManager->pauseSimulation();
        }
        else if (m_simManager->getStatus() == SimulationStatus::PAUSED)
        {
            m_simManager->runSimulation();
        }
    }
    break;

    // Quit Viewer
    case '\u001B':
    {
        m_simManager->getViewer()->endRenderingLoop();
    }break;

    }
}

void
VTKInteractorStyle::OnMouseMove()
{
    if (m_simManager->getStatus() != SimulationStatus::INACTIVE)
    {
        return;
    }

    vtkBaseInteractorStyle::OnMouseMove();
}

void
VTKInteractorStyle::OnLeftButtonDown()
{
    if (m_simManager->getStatus() != SimulationStatus::INACTIVE)
    {
        return;
    }

    vtkBaseInteractorStyle::OnLeftButtonDown();
}

void
VTKInteractorStyle::OnLeftButtonUp()
{
    if (m_simManager->getStatus() != SimulationStatus::INACTIVE)
    {
        return;
    }

    vtkBaseInteractorStyle::OnLeftButtonUp();
}

void
VTKInteractorStyle::OnMiddleButtonDown()
{
    if (m_simManager->getStatus() != SimulationStatus::INACTIVE)
    {
        return;
    }

    vtkBaseInteractorStyle::OnMiddleButtonDown();
}

void
VTKInteractorStyle::OnMiddleButtonUp()
{
    if (m_simManager->getStatus() != SimulationStatus::INACTIVE)
    {
        return;
    }

    vtkBaseInteractorStyle::OnMiddleButtonUp();
}

void
VTKInteractorStyle::OnRightButtonDown()
{
    if (m_simManager->getStatus() != SimulationStatus::INACTIVE)
    {
        return;
    }

    vtkBaseInteractorStyle::OnRightButtonDown();
}

void
VTKInteractorStyle::OnRightButtonUp()
{
    if (m_simManager->getStatus() != SimulationStatus::INACTIVE)
    {
        return;
    }

    vtkBaseInteractorStyle::OnRightButtonUp();
}

void
VTKInteractorStyle::OnMouseWheelForward()
{
    if (m_simManager->getStatus() != SimulationStatus::INACTIVE)
    {
        return;
    }

    vtkBaseInteractorStyle::OnMouseWheelForward();
}

void
VTKInteractorStyle::OnMouseWheelBackward()
{
    if (m_simManager->getStatus() != SimulationStatus::INACTIVE)
    {
        return;
    }

    vtkBaseInteractorStyle::OnMouseWheelBackward();
}

void
VTKInteractorStyle::setSimulationManager(SimulationManager *simManager)
{
    m_simManager = simManager;
}

double
VTKInteractorStyle::getTargetFrameRate() const
{
    if(m_targetMS == 0)
    {
        LOG(WARNING) << "VTKInteractorStyle::getTargetFrameRate warning: render target period is set to 0ms, "
                     << "therefore not regulated by a framerate. Returning 0.";
        return 0;
    }
    return 1000.0/m_targetMS;
}

void
VTKInteractorStyle::setTargetFrameRate(const double &fps)
{
    if(fps < 0)
    {
        LOG(WARNING) << "VTKInteractorStyle::setTargetFrameRate error: framerate must be positive, "
                     << "or equal to 0 to render as fast as possible.";
        return;
    }
    if(fps == 0)
    {
        m_targetMS = 0;
        return;
    }
    m_targetMS = 1000.0/fps;
    std::cout << "Target framerate: " << fps << " (" << m_targetMS << " ms)"<< std::endl;
}
} // imstk
