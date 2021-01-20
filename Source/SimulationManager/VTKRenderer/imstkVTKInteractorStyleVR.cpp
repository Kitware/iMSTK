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
#include "imstkOpenVRDeviceClient.h"

#include <vtkObjectFactory.h>
#include <vtkOpenVRRenderWindowInteractor.h>

vtkStandardNewMacro(vtkInteractorStyleVR);

vtkInteractorStyleVR::vtkInteractorStyleVR()
{
    // Setup the VR device clients
    m_leftControllerDeviceClient  = imstk::OpenVRDeviceClient::New(OPENVR_LEFT_CONTROLLER);
    m_rightControllerDeviceClient = imstk::OpenVRDeviceClient::New(OPENVR_RIGHT_CONTROLLER);
    m_hmdDeviceClient = imstk::OpenVRDeviceClient::New(OPENVR_HMD);
}

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
        m_leftControllerDeviceClient->setPose(pos, imstk::Quatd(imstk::Rotd(vtkMath::RadiansFromDegrees(orientation[0]),
            imstk::Vec3d(orientation[1], orientation[2], orientation[3]))));
    }
    else if (vtkEventDataDevice::RightController == eventDataDevice->GetDevice())
    {
        imstk::Vec3d pos;
        eventDataDevice->GetWorldPosition(pos.data());
        double orientation[4];
        eventDataDevice->GetWorldOrientation(orientation);
        m_rightControllerDeviceClient->setPose(pos, imstk::Quatd(imstk::Rotd(vtkMath::RadiansFromDegrees(orientation[0]),
            imstk::Vec3d(orientation[1], orientation[2], orientation[3]))));
    }
    else if (vtkEventDataDevice::HeadMountedDisplay == eventDataDevice->GetDevice())
    {
        imstk::Vec3d pos;
        eventDataDevice->GetWorldPosition(pos.data());
        double orientation[4];
        eventDataDevice->GetWorldOrientation(orientation);
        m_hmdDeviceClient->setPose(pos, imstk::Quatd(imstk::Rotd(vtkMath::RadiansFromDegrees(orientation[0]),
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
    vtkEventDataButton3D*         eventDataButton = static_cast<vtkEventDataButton3D*>(eventData);
    const vtkEventDataDevice      device = eventDataButton->GetDevice();
    const vtkEventDataAction      action = eventDataButton->GetAction();
    const vtkEventDataDeviceInput input  = eventDataButton->GetInput();

    switch (action)
    {
    case vtkEventDataAction::Press:
        if (device == vtkEventDataDevice::LeftController)
        {
            m_leftControllerDeviceClient->emitButtonPress(static_cast<int>(input));
        }
        else if (device == vtkEventDataDevice::RightController)
        {
            m_rightControllerDeviceClient->emitButtonPress(static_cast<int>(input));
        }
        break;
    case vtkEventDataAction::Release:
        if (device == vtkEventDataDevice::LeftController)
        {
            m_leftControllerDeviceClient->emitButtonRelease(static_cast<int>(input));
        }
        else if (device == vtkEventDataDevice::RightController)
        {
            m_rightControllerDeviceClient->emitButtonRelease(static_cast<int>(input));
        }
        break;
    case vtkEventDataAction::Touch:
        if (device == vtkEventDataDevice::LeftController)
        {
            m_leftControllerDeviceClient->emitButtonTouched(static_cast<int>(input));
        }
        else if (device == vtkEventDataDevice::RightController)
        {
            m_rightControllerDeviceClient->emitButtonTouched(static_cast<int>(input));
        }
        break;
    case vtkEventDataAction::Untouch:
        if (device == vtkEventDataDevice::LeftController)
        {
            m_leftControllerDeviceClient->emitButtonUntouched(static_cast<int>(input));
        }
        else if (device == vtkEventDataDevice::RightController)
        {
            m_rightControllerDeviceClient->emitButtonUntouched(static_cast<int>(input));
        }
        break;
    default:
        break;
    }
}