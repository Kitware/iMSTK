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

#include "imstkVTKOpenVRViewer.h"
#include "imstkDeviceControl.h"
#include "imstkLogger.h"
#include "imstkOpenVRDeviceClient.h"
#include "imstkScene.h"
#include "imstkVTKInteractorStyle.h"
#include "imstkVTKInteractorStyleVR.h"
#include "imstkVTKRenderer.h"

#include <vtkOpenVRRenderer.h>
#include <vtkOpenVRRenderWindow.h>
#include <vtkOpenVRRenderWindowInteractor.h>
#include <vtkOpenVRModel.h>

namespace imstk
{
VTKOpenVRViewer::VTKOpenVRViewer(std::string name) : AbstractVTKViewer(name)
{
    // Create the interactor style
    auto vrInteractorStyle = std::make_shared<vtkInteractorStyleVR>();
    m_interactorStyle    = std::dynamic_pointer_cast<InteractorStyle>(vrInteractorStyle);
    m_vtkInteractorStyle = std::dynamic_pointer_cast<vtkInteractorStyle>(m_interactorStyle);
    m_interactorStyle->setUpdateFunc([&]() { updateThread(); });

    // Create the interactor
    vtkNew<vtkOpenVRRenderWindowInteractor> iren;
    iren->SetInteractorStyle(m_vtkInteractorStyle.get());

    // Create the RenderWindow
    m_vtkRenderWindow = vtkSmartPointer<vtkOpenVRRenderWindow>::New();
    m_vtkRenderWindow->SetInteractor(iren);
    iren->SetRenderWindow(m_vtkRenderWindow);
    m_vtkRenderWindow->HideCursor();

    m_vrDeviceClients.push_back(vrInteractorStyle->getLeftControllerDeviceClient());
    m_vrDeviceClients.push_back(vrInteractorStyle->getRightControllerDeviceClient());
    m_vrDeviceClients.push_back(vrInteractorStyle->getHmdDeviceClient());
}

void
VTKOpenVRViewer::setActiveScene(const std::shared_ptr<Scene>& scene)
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
        m_rendererMap[m_activeScene] = std::make_shared<VTKRenderer>(m_activeScene, true);
    }

    // Cast to VTK renderer
    auto vtkRenderer = std::dynamic_pointer_cast<VTKRenderer>(this->getActiveRenderer())->getVtkRenderer();

    // Set renderer to renderWindow
    m_vtkRenderWindow->AddRenderer(vtkRenderer);

    m_vtkInteractorStyle->SetCurrentRenderer(vtkRenderer);
}

void
VTKOpenVRViewer::setRenderingMode(const Renderer::Mode mode)
{
    if (!m_activeScene)
    {
        LOG(WARNING) << "Missing scene, can not set rendering mode.\n"
                     << "Use Viewer::setCurrentScene to setup scene.";
        return;
    }

    // Setup renderer
    this->getActiveRenderer()->setMode(mode, true);
    if (m_status != ThreadStatus::Running)
    {
        return;
    }

    // Render to update displayed actors
    m_vtkRenderWindow->Render();
}

void
VTKOpenVRViewer::startThread()
{
    // Print all controls on viewer
    for (auto control : m_controls)
    {
        control->printControls();
    }

    // VR interactor doesn't support timers, here we throw timer event every update
    // another option would be to conform VTKs VR interactor
    vtkSmartPointer<vtkOpenVRRenderWindowInteractor> iren = vtkOpenVRRenderWindowInteractor::SafeDownCast(m_vtkRenderWindow->GetInteractor());
    //iren->Start(); // Cannot use
    if (iren->HasObserver(vtkCommand::StartEvent))
    {
        iren->InvokeEvent(vtkCommand::StartEvent, nullptr);
        return;
    }
    iren->Initialize();

    // Hide the device overlays
    // \todo: put in debug mode
    vtkSmartPointer<vtkOpenVRRenderWindow> renWin = vtkOpenVRRenderWindow::SafeDownCast(m_vtkRenderWindow);
    renWin->Initialize();
    renWin->Render(); // Must do one render to initialize vtkOpenVRModel's to then hide the controllers

    //renWin->GetTrackedDeviceModel(vr::trackeddevice)
    // Hide all controllers
    for (int i = 0; i < vr::k_unMaxTrackedDeviceCount; i++)
    {
        vtkOpenVRModel* trackedDeviceModel = renWin->GetTrackedDeviceModel(i);
        if (trackedDeviceModel != nullptr)
        {
            trackedDeviceModel->SetVisibility(false);
        }
    }

    while (!iren->GetDone())
    {
        auto vtkRen = std::dynamic_pointer_cast<VTKRenderer>(getActiveRenderer());
        iren->DoOneEvent(vtkOpenVRRenderWindow::SafeDownCast(m_vtkRenderWindow), vtkOpenVRRenderer::SafeDownCast(vtkRen->getVtkRenderer()));
        iren->InvokeEvent(vtkCommand::TimerEvent);
    }
}

std::shared_ptr<OpenVRDeviceClient>
VTKOpenVRViewer::getVRDeviceClient(int deviceType)
{
    auto iter = std::find_if(m_vrDeviceClients.begin(), m_vrDeviceClients.end(),
        [&](const std::shared_ptr<OpenVRDeviceClient>& deviceClient)
        {
            return static_cast<int>(deviceClient->getDeviceType()) == deviceType;
        });
    return (iter == m_vrDeviceClients.end()) ? nullptr : *iter;
}

void
VTKOpenVRViewer::updateThread()
{
    this->postEvent(Event(EventType::PreUpdate));

    // Update all controls
    for (auto control : m_controls)
    {
        control->update();
    }

    std::shared_ptr<imstk::VTKRenderer> renderer = std::dynamic_pointer_cast<imstk::VTKRenderer>(getActiveRenderer());
    if (renderer == nullptr)
    {
        return;
    }

    // Update Camera
    // \todo: No programmatic control over VR camera currently
    //renderer->updateSceneCamera(getActiveScene()->getCamera());

    // Update render delegates
    renderer->updateRenderDelegates();

    // Render
    m_vtkRenderWindow->GetInteractor()->Render();

    this->postEvent(Event(EventType::PostUpdate));

    // Plan next render
    m_vtkRenderWindow->GetInteractor()->CreateOneShotTimer(0);
}
}
