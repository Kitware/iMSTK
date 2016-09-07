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
#include "imstkInteractorStyle.h"
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

vtkStandardNewMacro(InteractorStyle);

void
InteractorStyle::OnTimer()
{
    if (m_simManager->getStatus() != SimulationStatus::RUNNING)
    {
        return;
    }

    // Update Camera
    auto scene = m_simManager->getCurrentScene();
    m_simManager->getViewer()->getCurrentRenderer()->updateSceneCamera(scene->getCamera());

    // Update render delegates
    m_simManager->getViewer()->getCurrentRenderer()->updateRenderDelegates();

    // Reset camera clipping range
    if(this->CurrentRenderer != nullptr)
    {
        this->CurrentRenderer->ResetCameraClippingRange();
    }

    // Render
    this->Interactor->Render();
}

void
InteractorStyle::OnChar()
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
InteractorStyle::OnMouseMove()
{
    if (m_simManager->getStatus() != SimulationStatus::INACTIVE)
    {
        return;
    }

    vtkBaseInteractorStyle::OnMouseMove();
}

void
InteractorStyle::OnLeftButtonDown()
{
    if (m_simManager->getStatus() != SimulationStatus::INACTIVE)
    {
        return;
    }

    vtkBaseInteractorStyle::OnLeftButtonDown();
}

void
InteractorStyle::OnLeftButtonUp()
{
    if (m_simManager->getStatus() != SimulationStatus::INACTIVE)
    {
        return;
    }

    vtkBaseInteractorStyle::OnLeftButtonUp();
}

void
InteractorStyle::OnMiddleButtonDown()
{
    if (m_simManager->getStatus() != SimulationStatus::INACTIVE)
    {
        return;
    }

    vtkBaseInteractorStyle::OnMiddleButtonDown();
}

void
InteractorStyle::OnMiddleButtonUp()
{
    if (m_simManager->getStatus() != SimulationStatus::INACTIVE)
    {
        return;
    }

    vtkBaseInteractorStyle::OnMiddleButtonUp();
}

void
InteractorStyle::OnRightButtonDown()
{
    if (m_simManager->getStatus() != SimulationStatus::INACTIVE)
    {
        return;
    }

    vtkBaseInteractorStyle::OnRightButtonDown();
}

void
InteractorStyle::OnRightButtonUp()
{
    if (m_simManager->getStatus() != SimulationStatus::INACTIVE)
    {
        return;
    }

    vtkBaseInteractorStyle::OnRightButtonUp();
}

void
InteractorStyle::OnMouseWheelForward()
{
    if (m_simManager->getStatus() != SimulationStatus::INACTIVE)
    {
        return;
    }

    vtkBaseInteractorStyle::OnMouseWheelForward();
}

void
InteractorStyle::OnMouseWheelBackward()
{
    if (m_simManager->getStatus() != SimulationStatus::INACTIVE)
    {
        return;
    }

    vtkBaseInteractorStyle::OnMouseWheelBackward();
}

void
InteractorStyle::setSimulationManager(SimulationManager *simManager)
{
    m_simManager = simManager;
}

} // imstk