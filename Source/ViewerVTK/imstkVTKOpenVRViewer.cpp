/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkVTKOpenVRViewer.h"
#include "imstkCamera.h"
#include "imstkDeviceControl.h"
#include "imstkLogger.h"
#include "imstkOpenVRDeviceClient.h"
#include "imstkScene.h"
#include "imstkVTKInteractorStyleVR.h"
#include "imstkVTKRenderer.h"

#include "imstkVtkOpenVRRenderWindowInteractorImstk.h"

#include <vtkMatrix4x4.h>
#include <vtkOpenVRRenderer.h>
#include <vtkOpenVRRenderWindow.h>
#include <vtkOpenVRModel.h>

namespace imstk
{
VTKOpenVRViewer::VTKOpenVRViewer(std::string name) : AbstractVTKViewer(name)
{
    // Create the interactor style
    auto vrInteractorStyle = vtkSmartPointer<vtkInteractorStyleVR>::New();
    m_vtkInteractorStyle = vrInteractorStyle;

    // Create the interactor
    auto iren = vtkSmartPointer<vtkOpenVRRenderWindowInteractorImstk>::New();
    iren->SetInteractorStyle(m_vtkInteractorStyle);

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
VTKOpenVRViewer::setActiveScene(std::shared_ptr<Scene> scene)
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
VTKOpenVRViewer::setPhysicalToWorldTransform(const Mat4d& physicalToWorldMatrix)
{
    vtkSmartPointer<vtkOpenVRRenderWindow> renWin =
        vtkOpenVRRenderWindow::SafeDownCast(m_vtkRenderWindow);
    vtkNew<vtkMatrix4x4> mat;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            mat->SetElement(i, j, physicalToWorldMatrix(i, j));
        }
    }
    renWin->SetPhysicalToWorldMatrix(mat);
}

Mat4d
VTKOpenVRViewer::getPhysicalToWorldTransform()
{
    vtkSmartPointer<vtkOpenVRRenderWindow> renWin =
        vtkOpenVRRenderWindow::SafeDownCast(m_vtkRenderWindow);
    Mat4d                transform;
    vtkNew<vtkMatrix4x4> mat;
    renWin->GetPhysicalToWorldMatrix(mat);
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            transform(i, j) = mat->GetElement(i, j);
        }
    }
    return transform;
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
}

void
VTKOpenVRViewer::processEvents()
{
    // Custom call to only process input events, do not perform a render
    auto iren = vtkOpenVRRenderWindowInteractorImstk::SafeDownCast(m_vtkRenderWindow->GetInteractor());
    auto ren  = std::dynamic_pointer_cast<imstk::VTKRenderer>(getActiveRenderer());
    iren->DoOneEvent(vtkOpenVRRenderWindow::SafeDownCast(m_vtkRenderWindow), ren->getVtkRenderer(), false);
}

bool
VTKOpenVRViewer::initModule()
{
    if (!AbstractVTKViewer::initModule())
    {
        return false;
    }

    // VR interactor doesn't support timers, here we throw timer event every update
    // another option would be to conform VTKs VR interactor
    auto iren = vtkOpenVRRenderWindowInteractorImstk::SafeDownCast(m_vtkRenderWindow->GetInteractor());
    //iren->Start(); // Cannot use
    if (iren->HasObserver(vtkCommand::StartEvent))
    {
        iren->InvokeEvent(vtkCommand::StartEvent, nullptr);
        return true;
    }

    auto renWin = vtkOpenVRRenderWindow::SafeDownCast(m_vtkRenderWindow);
    renWin->Initialize();

    iren->Initialize();

    // Hide the device overlays
    // \todo: Display devices in debug mode
    renWin->Render(); // Must do one render to initialize vtkOpenVRModel's to then hide the devices

    // Actions must be added after initialization of interactor
    vtkInteractorStyleVR* iStyle = vtkInteractorStyleVR::SafeDownCast(m_vtkInteractorStyle);
    iStyle->addButtonActions();
    iStyle->addMovementActions();

    // Hide all controller models
    for (uint32_t i = 0; i < vr::k_unMaxTrackedDeviceCount; i++)
    {
        vtkVRModel* trackedDeviceModel = renWin->GetTrackedDeviceModel(i);
        if (trackedDeviceModel != nullptr)
        {
            trackedDeviceModel->SetVisibility(false);
        }
    }

    return true;
}

void
VTKOpenVRViewer::updateModule()
{
    auto ren = std::dynamic_pointer_cast<imstk::VTKRenderer>(getActiveRenderer());
    if (ren == nullptr)
    {
        return;
    }

    // For the VR view we can't supply the a camera in the normal sense
    // we need to pre multiply a "user view"
    std::shared_ptr<Camera> cam  = getActiveScene()->getActiveCamera();
    const Mat4d&            view = cam->getView();
    setPhysicalToWorldTransform(view);

    // Update Camera
    // \todo: No programmatic control over VR camera currently
    //renderer->updateSceneCamera(getActiveScene()->getCamera());

    // Call visual update on every scene object
    getActiveScene()->updateVisuals(getDt());
    // Update all the rendering delegates
    ren->updateRenderDelegates();

    // Render
    //m_vtkRenderWindow->GetInteractor()->Render();
    m_vtkRenderWindow->Render();
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
} // namespace imstk
