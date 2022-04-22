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
#include "imstkDeviceControl.h"
#include "imstkLogger.h"
#include "imstkScene.h"
#include "imstkVTKInteractorStyle.h"
#include "imstkVTKRenderer.h"

#include <vtkCallbackCommand.h>
#include <vtkFileOutputWindow.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

namespace imstk
{
static void
exitCallbackFunc(
    vtkObject* vtkNotUsed(sender), unsigned long eventId,
    void* clientData, void* vtkNotUsed(callData))
{
    AbstractVTKViewer* viewer = static_cast<AbstractVTKViewer*>(clientData);
    if (viewer != nullptr
        && eventId == vtkCommand::ExitEvent)
    {
        viewer->pause(); // Immediately prevent any updates from running
        viewer->postEvent(Event(Module::end()));
    }
}

AbstractVTKViewer::AbstractVTKViewer(std::string name) : Viewer(name)
{
}

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
AbstractVTKViewer::setUseVsync(const bool useVsync)
{
    m_useVsync = useVsync;
    if (auto glRenWin = vtkOpenGLRenderWindow::SafeDownCast(m_vtkRenderWindow))
    {
        glRenWin->SetSwapControl(static_cast<int>(useVsync));
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
AbstractVTKViewer::setBackgroundColors(const Color color1, const Color color2, const bool gradientBackground)
{
    if (getActiveScene() == nullptr)
    {
        LOG(WARNING) << "Must set active scene before setting background color";
        return;
    }
    this->getActiveRenderer()->updateBackground(Vec3d(color1.r, color1.g, color1.b), Vec3d(color2.r, color2.g, color2.b), gradientBackground);
}

void
AbstractVTKViewer::processEvents()
{
    m_vtkRenderWindow->GetInteractor()->ProcessEvents();

    // Update all controls
    for (auto control : m_controls)
    {
        control->update(m_dt);
    }
}

void
AbstractVTKViewer::setVtkLoggerMode(VTKLoggerMode loggerMode)
{
    if (loggerMode == VTKLoggerMode::SHOW)
    {
        vtkObject::GlobalWarningDisplayOn();
    }
    else if (loggerMode == VTKLoggerMode::MUTE)
    {
        vtkObject::GlobalWarningDisplayOff();
    }
    else if (loggerMode == VTKLoggerMode::WRITE)
    {
        vtkObject::GlobalWarningDisplayOn();

        // Set the VTK output window to log
        vtkNew<vtkFileOutputWindow> fileOutputWindow;
        fileOutputWindow->SetFileName("vtkOutput.log");
        vtkOutputWindow::SetInstance(fileOutputWindow);
    }
}

bool
AbstractVTKViewer::initModule()
{
    // Print all controls on viewer
    for (auto control : m_controls)
    {
        control->printControls();
    }

    exitCallback = vtkSmartPointer<vtkCallbackCommand>::New();
    exitCallback->SetCallback(exitCallbackFunc);
    exitCallback->SetClientData(this);
    m_vtkRenderWindow->GetInteractor()->AddObserver(vtkCommand::ExitEvent, exitCallback);
    return true;
}

void
AbstractVTKViewer::uninitModule()
{ 
    auto iren = m_vtkRenderWindow->GetInteractor();
    iren->SetDone(true);
    m_vtkRenderWindow->Finalize();

    iren->TerminateApp();
    iren->ProcessEvents();
}
} // namespace imstk
