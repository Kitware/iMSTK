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

#include "imstkVTKInteractorStyleVR.h"
#include "imstkScene.h"
#include "imstkSimulationManager.h"
#include "imstkVTKRenderer.h"
#include "imstkVTKViewer.h"

#include <vtkObjectFactory.h>
#include <vtkOpenVRRenderWindowInteractor.h>
#include <vtkRenderer.h>

vtkStandardNewMacro(vtkInteractorStyleVR);

void
vtkInteractorStyleVR::OnMove3D(vtkEventData* eventData)
{
    if (eventData->GetType() != vtkCommand::Move3DEvent)
    {
        return;
    }
    vtkEventDataDevice3D* eventDataDevice = static_cast<vtkEventDataDevice3D*>(eventData);
    if (vtkEventDataDevice::LeftController == eventDataDevice->GetDevice())
    {
        imstk::Vec3d pos;
        eventDataDevice->GetWorldPosition(pos.data());
        double orientation[4];
        eventDataDevice->GetWorldOrientation(orientation);
        leftControllerPoseChanged.emit(pos, imstk::Quatd(imstk::Rotd(vtkMath::RadiansFromDegrees(orientation[0]),
                        imstk::Vec3d(orientation[1], orientation[2], orientation[3]))));
    }
    else if (vtkEventDataDevice::RightController == eventDataDevice->GetDevice())
    {
        imstk::Vec3d pos;
        eventDataDevice->GetWorldPosition(pos.data());
        double orientation[4];
        eventDataDevice->GetWorldOrientation(orientation);
        rightControllerPoseChanged.emit(pos, imstk::Quatd(imstk::Rotd(vtkMath::RadiansFromDegrees(orientation[0]),
                        imstk::Vec3d(orientation[1], orientation[2], orientation[3]))));
    }
    else if (vtkEventDataDevice::HeadMountedDisplay == eventDataDevice->GetDevice())
    {
        imstk::Vec3d pos;
        eventDataDevice->GetWorldPosition(pos.data());
        double orientation[4];
        eventDataDevice->GetWorldOrientation(orientation);
        hmdPoseChanged.emit(pos, imstk::Quatd(imstk::Rotd(vtkMath::RadiansFromDegrees(orientation[0]),
                        imstk::Vec3d(orientation[1], orientation[2], orientation[3]))));
    }
}

void
vtkInteractorStyleVR::OnButton3D(vtkEventData* eventData)
{
    if (eventData->GetType() != vtkCommand::Button3DEvent)
    {
        return;
    }
    vtkEventDataButton3D* eventDataButton = static_cast<vtkEventDataButton3D*>(eventData);
    //printf("Button: %d\n", eventDataButton->GetInput());
    //bd->GetAction() == vtkEventDataAction::Press
}

void
vtkInteractorStyleVR::OnTimer()
{
    auto renderer = std::static_pointer_cast<imstk::VTKRenderer>(m_simManager->getViewer()->getActiveRenderer());

    // Update Camera
    renderer->updateSceneCamera(m_simManager->getActiveScene()->getCamera());

    // Update render delegates
    renderer->updateRenderDelegates();

    // Reset camera clipping range
    this->CurrentRenderer->ResetCameraClippingRange();

    // Render
    this->Interactor->Render();

    // Plan next render
    this->Interactor->CreateOneShotTimer(0);
}