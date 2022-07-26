/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkVTKInteractorStyleVR.h"
#include "imstkLogger.h"
#include "imstkOpenVRDeviceClient.h"

#include <vtkMath.h>
#include <vtkEventData.h>
#include <vtkObjectFactory.h>
#include "imstkVtkOpenVRRenderWindowInteractorImstk.h"

vtkStandardNewMacro(vtkInteractorStyleVR);

vtkInteractorStyleVR::vtkInteractorStyleVR()
{
    // Setup the VR device clients
    m_leftControllerDeviceClient  = imstk::OpenVRDeviceClient::New(OPENVR_LEFT_CONTROLLER);
    m_rightControllerDeviceClient = imstk::OpenVRDeviceClient::New(OPENVR_RIGHT_CONTROLLER);
    m_hmdDeviceClient = imstk::OpenVRDeviceClient::New(OPENVR_HMD);
}

void
vtkInteractorStyleVR::OnButtonPress(vtkEventData* data, int buttonId)
{
    vtkEventDataForDevice*   eventDataButton = data->GetAsEventDataForDevice();
    const vtkEventDataAction action = eventDataButton->GetAction();
    const vtkEventDataDevice device = eventDataButton->GetDevice();

    if (device == vtkEventDataDevice::LeftController)
    {
        if (action == vtkEventDataAction::Press)
        {
            m_leftControllerDeviceClient->emitButtonPress(buttonId);
        }
        else if (action == vtkEventDataAction::Release)
        {
            m_leftControllerDeviceClient->emitButtonRelease(buttonId);
        }
    }
    else if (device == vtkEventDataDevice::RightController)
    {
        if (action == vtkEventDataAction::Press)
        {
            m_rightControllerDeviceClient->emitButtonPress(buttonId);
        }
        else if (action == vtkEventDataAction::Release)
        {
            m_rightControllerDeviceClient->emitButtonRelease(buttonId);
        }
    }
}

void
vtkInteractorStyleVR::addMovementActions()
{
    vtkOpenVRRenderWindowInteractorImstk* iren =
        vtkOpenVRRenderWindowInteractorImstk::SafeDownCast(GetInteractor());
    CHECK(iren->GetInitialized()) << "Cannot addMovementActions to style until "
        "interactor has been initialized";
    iren->AddAction("/actions/vtk/in/LeftGripMovement", true,
        [this](vtkEventData* ed)
        {
            vtkEventDataDevice3D* edd = ed->GetAsEventDataDevice3D();
            const double* pos = edd->GetTrackPadPosition();
            m_leftControllerDeviceClient->setTrackpadPosition(imstk::Vec2d(pos[0], pos[1]));
        });
    iren->AddAction("/actions/vtk/in/RightGripMovement", true,
        [this](vtkEventData* ed)
        {
            vtkEventDataDevice3D* edd = ed->GetAsEventDataDevice3D();
            const double* pos = edd->GetTrackPadPosition();
            m_rightControllerDeviceClient->setTrackpadPosition(imstk::Vec2d(pos[0], pos[1]));
        });
}

void
vtkInteractorStyleVR::addButtonActions()
{
    vtkOpenVRRenderWindowInteractorImstk* iren =
        vtkOpenVRRenderWindowInteractorImstk::SafeDownCast(GetInteractor());
    CHECK(iren->GetInitialized()) << "Cannot addButtonActions to style until "
        "interactor has been initialized";

    // Called when buttons are pressed/released
    std::array<std::string, 6> buttonActionNames =
    {
        "/actions/vtk/in/Button0Pressed",
        "/actions/vtk/in/Button1Pressed",
        "/actions/vtk/in/Button2Pressed",
        "/actions/vtk/in/Button3Pressed",
        "/actions/vtk/in/GripPressed",
        "/actions/vtk/in/TriggerPressed"
    };
    for (int i = 0; i < 6; i++)
    {
        iren->AddAction(buttonActionNames[i], false,
            [this, i](vtkEventData* ed)
            {
                OnButtonPress(ed, i);
            });
    }
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