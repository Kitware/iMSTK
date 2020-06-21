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

#include "imstkVTKViewer.h"
#include "imstkVTKRenderDelegate.h"
#include "imstkVTKInteractorStyle.h"
#include "imstkVTKRenderer.h"
#include "imstkCamera.h"
#include "imstkLogger.h"
#include "imstkScene.h"

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCallbackCommand.h>

#include "imstkParallelUtils.h"

namespace imstk
{
VTKViewer::VTKViewer(SimulationManager* manager /*= nullptr*/, bool enableVR /*= false*/) :
    m_enableVR(enableVR)
{
    // init render window / interactor / command based
    // depending on if we enable VR or not
    if (!m_enableVR)
    {
        // Interactor style / commands
        m_interactorStyle = std::make_shared<VTKInteractorStyle>();

        m_vtkInteractorStyle = std::dynamic_pointer_cast<VTKInteractorStyle>(m_interactorStyle);
        m_vtkInteractorStyle->m_simManager = manager;

        // Interactor
        auto vtkInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
        vtkInteractor->SetInteractorStyle(m_vtkInteractorStyle.get());

        // Render window
        m_vtkRenderWindow = vtkSmartPointer<vtkRenderWindow>::New();
        m_vtkRenderWindow->SetInteractor(vtkInteractor);
        m_vtkRenderWindow->SetSize(m_config->m_renderWinWidth, m_config->m_renderWinHeight);

        // Screen capture
        m_screenCapturer = std::make_shared<VTKScreenCaptureUtility>(m_vtkRenderWindow);
    }
#ifdef iMSTK_ENABLE_VR
    else
    {
        // Interactor style / commands
        m_openVRCommand = vtkSmartPointer<OpenVRCommand>::New();
        m_openVRCommand->SetSimulationManager(manager);

        // Interactor
        auto vtkOpenVRinteractor = vtkSmartPointer<vtkOpenVRRenderWindowInteractor>::New();

        // Add observer openVR command
        m_vtkRenderWindow = vtkSmartPointer<vtkOpenVRRenderWindow>::New();
        m_vtkRenderWindow->SetInteractor(vtkOpenVRinteractor);
        vtkOpenVRinteractor->SetRenderWindow(m_vtkRenderWindow);
        m_vtkRenderWindow->AddObserver(vtkCommand::StartEvent, m_openVRCommand, 1.0);
    }
#endif

    // Setup callback for timer on the interactor
    timerCallbackCommand = vtkSmartPointer<vtkCallbackCommand>::New();
    timerCallbackCommand->SetCallback(timerCallback);
    timerCallbackCommand->SetClientData(this);
}

void
VTKViewer::setActiveScene(const std::shared_ptr<Scene>& scene)
{
    // If already current scene
    if (scene == m_activeScene)
    {
        LOG(WARNING) << scene->getName() << " already is the viewer current scene.";
        return;
    }

    // If the current scene has a renderer, remove it
    if (m_activeScene)
    {
        auto vtkRenderer = std::dynamic_pointer_cast<VTKRenderer>(this->getActiveRenderer())->getVtkRenderer();
        if (m_vtkRenderWindow->HasRenderer(vtkRenderer))
        {
            m_vtkRenderWindow->RemoveRenderer(vtkRenderer);
        }
    }

    // Update current scene
    m_activeScene = scene;

    // Create renderer if it doesn't exist
    if (!m_rendererMap.count(m_activeScene))
    {
        m_rendererMap[m_activeScene] = std::make_shared<VTKRenderer>(m_activeScene, m_enableVR);
    }

    // Cast to VTK renderer
    auto vtkRenderer = std::dynamic_pointer_cast<VTKRenderer>(this->getActiveRenderer())->getVtkRenderer();

    // Set renderer to renderWindow
    m_vtkRenderWindow->AddRenderer(vtkRenderer);

    if (!m_enableVR)
    {
        // Set renderer to interactorStyle
        std::dynamic_pointer_cast<VTKInteractorStyle>(m_interactorStyle)->SetCurrentRenderer(vtkRenderer);

        // Set name to renderWindow
        m_vtkRenderWindow->SetWindowName(m_activeScene->getName().data());
    }
}

void
VTKViewer::setRenderingMode(const Renderer::Mode mode)
{
    if (!m_activeScene)
    {
        LOG(WARNING) << "Missing scene, can not set rendering mode.\n"
                     << "Use Viewer::setCurrentScene to setup scene.";
        return;
    }

    // Setup renderer
    this->getActiveRenderer()->setMode(mode, m_enableVR);
    if (!m_running)
    {
        return;
    }

    // Render to update displayed actors
    m_vtkRenderWindow->Render();

    if (m_enableVR)
    {
        return;
    }

    // Setup render window
    //std::dynamic_pointer_cast<VTKInteractorStyle>(m_interactorStyle)->HighlightProp(nullptr);

    if (m_config->m_hideCurzor)
    {
        m_vtkRenderWindow->HideCursor();
    }

    if (m_config->m_hideBorder)
    {
        m_vtkRenderWindow->BordersOff();
    }

    if (m_config->m_fullScreen)
    {
        m_vtkRenderWindow->FullScreenOn();
    }
}

Renderer::Mode
VTKViewer::getRenderingMode()
{
    return this->getActiveRenderer()->getMode();
}

void
VTKViewer::startRenderingLoop()
{
    m_running = true;
    if (!m_enableVR)
    {
        m_vtkRenderWindow->GetInteractor()->Initialize();
        m_vtkRenderWindow->GetInteractor()->CreateOneShotTimer(0);

        // If the Scene wants benchmarking hookup timer to update the table
        auto vtkRen = std::dynamic_pointer_cast<VTKRenderer>(getActiveRenderer());
        vtkRen->setTimeTableVisibility(m_activeScene->getConfig()->taskTimingEnabled);
        if (m_activeScene->getConfig()->taskTimingEnabled)
        {
            m_vtkRenderWindow->GetInteractor()->AddObserver(vtkCommand::TimerEvent, timerCallbackCommand);
            m_vtkRenderWindow->GetInteractor()->CreateRepeatingTimer(500);
        }

        m_vtkRenderWindow->SetWindowName(m_config->m_windowName.c_str());
        m_vtkRenderWindow->GetInteractor()->Start();
        m_vtkRenderWindow->GetInteractor()->DestroyTimer();
    }
    else
    {
        m_vtkRenderWindow->GetInteractor()->Start();
    }

    m_running = false;
}

void
VTKViewer::endRenderingLoop()
{
    // close the rendering window
    m_vtkRenderWindow->Finalize();

    // Terminate the interactor
    m_vtkRenderWindow->GetInteractor()->TerminateApp();
}

vtkSmartPointer<vtkRenderWindow>
VTKViewer::getVtkRenderWindow() const
{
    return m_vtkRenderWindow;
}

std::shared_ptr<VTKScreenCaptureUtility>
VTKViewer::getScreenCaptureUtility() const
{
    return std::static_pointer_cast<VTKScreenCaptureUtility>(m_screenCapturer);
}

void
VTKViewer::setBackgroundColors(const Vec3d color1, const Vec3d color2 /*= Vec3d::Zero()*/, const bool gradientBackground /*= false*/)
{
    this->getActiveRenderer()->updateBackground(color1, color2, gradientBackground);
}

void
VTKViewer::setWindowTitle(const std::string& title)
{
    m_config->m_windowName = title;
    if (m_vtkRenderWindow)
    {
        m_vtkRenderWindow->SetWindowName(title.c_str());
    }
}

const std::shared_ptr<VTKTextStatusManager>&
VTKViewer::getTextStatusManager()
{
    return m_vtkInteractorStyle->getTextStatusManager();
}

void
VTKViewer::timerCallback(vtkObject* caller, long unsigned int vtkNotUsed(eventId), void* clientData, void* vtkNotUsed(callData))
{
    VTKViewer* self = static_cast<VTKViewer*>(clientData);

    if (self->getActiveScene()->getConfig()->taskTimingEnabled)
    {
        auto vtkRen = std::dynamic_pointer_cast<VTKRenderer>(self->getActiveRenderer());
        self->getActiveScene()->lockComputeTimes();
        vtkRen->setTimeTable(self->getActiveScene()->getTaskComputeTimes());
        self->getActiveScene()->unlockComputeTimes();
    }
}
} // imstk
