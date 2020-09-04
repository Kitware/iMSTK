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

#include "imstkAbstractVTKViewer.h"
#include "imstkLogger.h"
#include "imstkScene.h"
#include "imstkVTKInteractorStyle.h"
#include "imstkVTKRenderer.h"

#include <vtkCallbackCommand.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCallbackCommand.h>

namespace imstk
{
AbstractVTKViewer::AbstractVTKViewer(std::string name) : Viewer(name)
{
    viewerDisabledCallback = vtkSmartPointer<vtkCallbackCommand>::New();
    viewerDisabledCallback->SetCallback(viewerDisabled);
    viewerDisabledCallback->SetClientData(this);
    viewerEnabledCallback = vtkSmartPointer<vtkCallbackCommand>::New();
    viewerEnabledCallback->SetCallback(viewerEnabled);
    viewerEnabledCallback->SetClientData(this);
}

Renderer::Mode
AbstractVTKViewer::getRenderingMode() const
{
    return this->getActiveRenderer()->getMode();
}

void
AbstractVTKViewer::stopThread()
{
    // close the rendering window
    m_vtkRenderWindow->Finalize();

    // Terminate the interactor
    m_vtkRenderWindow->GetInteractor()->TerminateApp();
}

void
AbstractVTKViewer::pause(bool sync)
{
    // It doesn't actually stop until disable event
    if (m_status == ThreadStatus::Inactive)
    {
        return;
    }

    emit(Event(EventType::Pause));

    pauseThread();

    if (m_status == ThreadStatus::Running)
    {
        // Request that it disables

        // If sync, wait until the module actually pauses before returning from function
        if (sync)
        {
            m_vtkRenderWindow->GetInteractor()->AddObserver(vtkCallbackCommand::DisableEvent, viewerDisabledCallback);
            m_vtkRenderWindow->GetInteractor()->Disable();
            while (m_status != ThreadStatus::Paused) { }
            m_vtkRenderWindow->GetInteractor()->RemoveObserver(viewerDisabledCallback);
        }
        else
        {
            m_vtkRenderWindow->GetInteractor()->Disable();
        }
    }
}

void
AbstractVTKViewer::resume(bool sync)
{
    if (m_status == ThreadStatus::Inactive)
    {
        return;
    }

    emit(Event(EventType::Resume));

    resumeThread();
    if (m_status == ThreadStatus::Paused)
    {
        m_requestedStatus = ThreadStatus::Running;
        // If sync, wait until the module actually pauses before returning from function
        if (sync)
        {
            m_vtkRenderWindow->GetInteractor()->AddObserver(vtkCallbackCommand::EnableEvent, viewerEnabledCallback);
            m_vtkRenderWindow->GetInteractor()->Disable();
            while (m_status != ThreadStatus::Running) { }
            m_vtkRenderWindow->GetInteractor()->RemoveObserver(viewerEnabledCallback);
        }
        else
        {
            m_vtkRenderWindow->GetInteractor()->Disable();
        }
    }
}

void
AbstractVTKViewer::setSize(const int width, const int height)
{
    if (m_vtkRenderWindow != NULL)
    {
        m_vtkRenderWindow->SetSize(width, height);
    }
}

void
AbstractVTKViewer::setWindowTitle(const std::string& title)
{
    m_config->m_windowName = title;
    if (m_vtkRenderWindow)
    {
        m_vtkRenderWindow->SetWindowName(title.c_str());
    }
}

void
AbstractVTKViewer::setBackgroundColors(const Vec3d color1, const Vec3d color2 /*= Vec3d::Zero()*/, const bool gradientBackground /*= false*/)
{
    this->getActiveRenderer()->updateBackground(color1, color2, gradientBackground);
}

void
AbstractVTKViewer::viewerDisabled(vtkObject* vtkNotUsed(sender),
                                  unsigned long vtkNotUsed(eventId), void* clientData, void* vtkNotUsed(callData))
{
    imstk::AbstractVTKViewer* viewer = static_cast<imstk::AbstractVTKViewer*>(clientData);
    viewer->m_status = ThreadStatus::Paused;
}

void
AbstractVTKViewer::viewerEnabled(vtkObject* vtkNotUsed(sender),
                                 unsigned long vtkNotUsed(eventId), void* clientData, void* vtkNotUsed(callData))
{
    imstk::AbstractVTKViewer* viewer = static_cast<imstk::AbstractVTKViewer*>(clientData);
    viewer->m_status = ThreadStatus::Running;
}
}
