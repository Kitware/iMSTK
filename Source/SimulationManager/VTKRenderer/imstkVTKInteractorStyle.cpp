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

#include "imstkVTKInteractorStyle.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkMouseDeviceClient.h"

#include <vtkObjectFactory.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>

namespace imstk
{
vtkStandardNewMacro(VTKInteractorStyle);

VTKInteractorStyle::VTKInteractorStyle() :
    m_keyboardDeviceClient(KeyboardDeviceClient::New()),
    m_mouseDeviceClient(MouseDeviceClient::New())
{
}

VTKInteractorStyle::~VTKInteractorStyle()
{
    this->SetReferenceCount(0);
}

void
VTKInteractorStyle::OnTimer()
{
    m_updateFunc();
}

void
VTKInteractorStyle::OnKeyPress()
{
    // Submit the change to the keyboard device client
    m_keyboardDeviceClient->emitKeyDown(this->Interactor->GetKeyCode());
}

void
VTKInteractorStyle::OnKeyRelease()
{
    // Submit the change to the keyboard device client
    m_keyboardDeviceClient->emitKeyUp(this->Interactor->GetKeyCode());
}

void
VTKInteractorStyle::OnMouseMove()
{
    Vec2i mousePos;
    this->Interactor->GetEventPosition(mousePos[0], mousePos[1]);
    Vec2i dim;
    Interactor->GetSize(dim[0], dim[1]);
    m_mouseDeviceClient->updateMousePos(mousePos.cast<double>().cwiseQuotient(dim.cast<double>()));
}

void
VTKInteractorStyle::OnLeftButtonDown()
{
    m_mouseDeviceClient->emitButtonPress(LEFT_BUTTON);
}

void
VTKInteractorStyle::OnLeftButtonUp()
{
    m_mouseDeviceClient->emitButtonRelease(LEFT_BUTTON);
}

void
VTKInteractorStyle::OnMiddleButtonDown()
{
    m_mouseDeviceClient->emitButtonPress(MIDDLE_BUTTON);
}

void
VTKInteractorStyle::OnMiddleButtonUp()
{
    m_mouseDeviceClient->emitButtonRelease(MIDDLE_BUTTON);
}

void
VTKInteractorStyle::OnRightButtonDown()
{
    m_mouseDeviceClient->emitButtonPress(RIGHT_BUTTON);
}

void
VTKInteractorStyle::OnRightButtonUp()
{
    m_mouseDeviceClient->emitButtonRelease(RIGHT_BUTTON);
}

void
VTKInteractorStyle::OnMouseWheelForward()
{
    // \todo: VTK provides no scroll amount?
    m_mouseDeviceClient->emitScroll(-0.2);
}

void
VTKInteractorStyle::OnMouseWheelBackward()
{
    // \todo: VTK provides no scroll amount?
    m_mouseDeviceClient->emitScroll(0.2);
}
}
