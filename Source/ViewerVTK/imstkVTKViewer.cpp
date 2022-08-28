/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkVTKViewer.h"
#include "imstkLogger.h"
#include "imstkScene.h"
#include "imstkVTKInteractorStyle.h"
#include "imstkVTKRenderer.h"
#include "imstkVTKScreenCaptureUtility.h"

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkTextActor.h>
#include <vtkOpenGLRenderWindow.h>

#include <vtkCamera.h>

#ifdef WIN32
#include <vtkWin32HardwareWindow.h>
#include <vtkWin32RenderWindowInteractor.h>
#else
#ifdef iMSTK_USE_VTK_OSMESA
#include <vtkGenericRenderWindowInteractor.h>
#else
#include <vtkXRenderWindowInteractor.h>
#endif
#endif

namespace imstk
{
VTKViewer::VTKViewer(std::string name) : AbstractVTKViewer(name),
    m_lastFpsUpdate(std::chrono::high_resolution_clock::now()),
    m_lastFps(60.0)
{
    // Create the interactor style
    m_vtkInteractorStyle = vtkSmartPointer<VTKInteractorStyle>::New();

    // Create the interactor
#ifdef WIN32
    auto iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
#else
#ifdef iMSTK_USE_VTK_OSMESA
    auto iren = vtkSmartPointer<vtkGenericRenderWindowInteractor>::New();
#else
    vtkSmartPointer<vtkXRenderWindowInteractor> iren = vtkSmartPointer<vtkXRenderWindowInteractor>::New();
    iren->SetInteractorStyle(m_vtkInteractorStyle.Get());
#endif
#endif
    iren->SetInteractorStyle(m_vtkInteractorStyle);

    // Create the RenderWindow
    m_vtkRenderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    m_vtkRenderWindow->SetInteractor(iren);
    m_vtkRenderWindow->SetSize(m_config->m_renderWinWidth, m_config->m_renderWinHeight);

    // Screen capture
    m_screenCapturer = std::make_shared<VTKScreenCaptureUtility>(m_vtkRenderWindow);
}

void
VTKViewer::setActiveScene(std::shared_ptr<Scene> scene)
{
    // This function could be called before or after the viewer & renderer
    // have even started.

    // If already current scene
    if (scene == m_activeScene)
    {
        LOG(WARNING) << scene->getName() << " already is the viewer current scene.";
        return;
    }

    // If the current scene has a renderer, remove it
    if (m_activeScene != nullptr)
    {
        auto vtkRenderer = getActiveVtkRenderer()->getVtkRenderer();
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
        m_rendererMap[m_activeScene] = std::make_shared<VTKRenderer>(m_activeScene, false);
    }

    // Cast to VTK renderer
    auto vtkRenderer = getActiveVtkRenderer()->getVtkRenderer();

    // Set renderer to renderWindow
    m_vtkRenderWindow->AddRenderer(vtkRenderer);

    m_vtkInteractorStyle->SetCurrentRenderer(vtkRenderer);

    // Set name to renderWindow
    m_vtkRenderWindow->SetWindowName(m_activeScene->getName().data());
}

void
VTKViewer::setDebugAxesLength(double x, double y, double z)
{
    getActiveVtkRenderer()->setAxesLength(x, y, z);
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

    // Switch the renderer to the mode
    this->getActiveRenderer()->setMode(mode, false);

    updateModule();

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

bool
VTKViewer::initModule()
{
    if (!AbstractVTKViewer::initModule())
    {
        return false;
    }

    std::shared_ptr<VTKRenderer> ren = getActiveVtkRenderer();
    ren->initialize();

    // Update the camera
    if (ren != nullptr)
    {
        ren->updateCamera();
    }

    m_vtkRenderWindow->SetWindowName(m_config->m_windowName.c_str());
    if (m_vtkRenderWindow->GetInteractor()->HasObserver(vtkCommand::StartEvent))
    {
        m_vtkRenderWindow->GetInteractor()->InvokeEvent(vtkCommand::StartEvent, nullptr);
    }

    m_vtkRenderWindow->GetInteractor()->Initialize();
    setUseVsync(m_useVsync);
    m_vtkRenderWindow->Render();

    return true;
}

std::shared_ptr<VTKScreenCaptureUtility>
VTKViewer::getScreenCaptureUtility() const
{
    return std::static_pointer_cast<VTKScreenCaptureUtility>(m_screenCapturer);
}

std::shared_ptr<KeyboardDeviceClient>
VTKViewer::getKeyboardDevice() const
{
    return VTKInteractorStyle::SafeDownCast(m_vtkInteractorStyle)->getKeyboardDeviceClient();
}

std::shared_ptr<MouseDeviceClient>
VTKViewer::getMouseDevice() const
{
    return VTKInteractorStyle::SafeDownCast(m_vtkInteractorStyle)->getMouseDeviceClient();
}

void
VTKViewer::updateModule()
{
    std::shared_ptr<VTKRenderer> ren = std::dynamic_pointer_cast<VTKRenderer>(getActiveRenderer());
    if (ren == nullptr)
    {
        return;
    }

    // Update Camera
    ren->updateCamera();

    // Call visual update on every scene object
    getActiveScene()->updateVisuals(getDt());
    // Update all the rendering delegates
    ren->updateRenderDelegates();

    // Automatically determine near and far planes (not used atm)
    //ren->getVtkRenderer()->ResetCameraClippingRange();

    // Update framerate value display
    auto now = std::chrono::high_resolution_clock::now();
    m_visualFps = 1e6 / static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(now - m_pre).count());
    m_visualFps = 0.1 * m_visualFps + 0.9 * m_lastFps;
    m_lastFps   = m_visualFps;

    m_pre = now;

    // Render
    m_vtkRenderWindow->Render();
}
} // namespace imstk
