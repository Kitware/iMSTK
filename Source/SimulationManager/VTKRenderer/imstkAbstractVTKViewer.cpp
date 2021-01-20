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
static void exitCallbackFunc(
    vtkObject* vtkNotUsed(sender), unsigned long eventId,
    void* clientData, void* vtkNotUsed(callData))
{
    AbstractVTKViewer* viewer = static_cast<AbstractVTKViewer*>(clientData);
    if (viewer != nullptr &&
        eventId == vtkCommand::ExitEvent)
    {
        viewer->pause(); // Immediately prevent any updates from running
        viewer->postEvent(Event(EventType::End));
    }
}

AbstractVTKViewer::AbstractVTKViewer(std::string name) : Viewer(name) { }

Renderer::Mode
AbstractVTKViewer::getRenderingMode() const
{
    return this->getActiveRenderer()->getMode();
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
    if (getActiveScene() == nullptr)
    {
        LOG(WARNING) << "Must set active scene before setting background color";
        return;
    }
    this->getActiveRenderer()->updateBackground(color1, color2, gradientBackground);
}

void
AbstractVTKViewer::processEvents()
{
    m_vtkRenderWindow->GetInteractor()->ProcessEvents();
}

bool
AbstractVTKViewer::initModule()
{
    exitCallback = vtkSmartPointer<vtkCallbackCommand>::New();
    exitCallback->SetCallback(exitCallbackFunc);
    exitCallback->SetClientData(this);
    m_vtkRenderWindow->GetInteractor()->AddObserver(vtkCommand::ExitEvent, exitCallback);
    return true;
}

void
AbstractVTKViewer::uninitModule()
{
    // close the rendering window
    m_vtkRenderWindow->Finalize();

    // Terminate the interactor
    m_vtkRenderWindow->GetInteractor()->TerminateApp();
}
}
