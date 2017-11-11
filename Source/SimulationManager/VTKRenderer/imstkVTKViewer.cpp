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

namespace imstk
{
void
VTKViewer::setActiveScene(std::shared_ptr<Scene>scene)
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
        m_rendererMap[m_activeScene] = std::make_shared<VTKRenderer>(m_activeScene);
    }

    // Cast to VTK renderer
    auto vtkRenderer = std::dynamic_pointer_cast<VTKRenderer>(this->getActiveRenderer())->getVtkRenderer();

    // Set renderer to renderWindow
    m_vtkRenderWindow->AddRenderer(vtkRenderer);

    // Set renderer to interactorStyle
    std::dynamic_pointer_cast<VTKInteractorStyle>(m_interactorStyle)->SetCurrentRenderer(vtkRenderer);

    // Set name to renderWindow
    m_vtkRenderWindow->SetWindowName(m_activeScene->getName().data());
}

void
VTKViewer::setRenderingMode(Renderer::Mode mode)
{
    if (!m_activeScene)
    {
        LOG(WARNING) << "Missing scene, can not set rendering mode.\n"
                     << "Use Viewer::setCurrentScene to setup scene.";
        return;
    }

    // Setup renderer
    this->getActiveRenderer()->setMode(mode);
    if (!m_running)
    {
        return;
    }

    // Render to update displayed actors
    m_vtkRenderWindow->Render();

    // Setup render window
    if (mode == Renderer::Mode::SIMULATION)
    {
        std::dynamic_pointer_cast<VTKInteractorStyle>(m_interactorStyle)->HighlightProp(nullptr);
        m_vtkRenderWindow->HideCursor();
        //m_vtkRenderWindow->BordersOff();
        //m_vtkRenderWindow->FullScreenOn(1);
    }
    else
    {
        m_vtkRenderWindow->ShowCursor();
        //m_vtkRenderWindow->BordersOn();
        //m_vtkRenderWindow->FullScreenOff(1);
    }
}

const Renderer::Mode
VTKViewer::getRenderingMode()
{
    return this->getActiveRenderer()->getMode();
}

void
VTKViewer::startRenderingLoop()
{
    m_running = true;
    m_vtkRenderWindow->GetInteractor()->Initialize();
    m_vtkRenderWindow->GetInteractor()->CreateOneShotTimer(0);
    m_vtkRenderWindow->GetInteractor()->Start();
    m_vtkRenderWindow->GetInteractor()->DestroyTimer();
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

double
VTKViewer::getTargetFrameRate() const
{
    auto vtkInteractorStyle = std::dynamic_pointer_cast<VTKInteractorStyle>(m_interactorStyle);
    if(vtkInteractorStyle->m_targetMS == 0)
    {
        LOG(WARNING) << "VTKViewer::getTargetFrameRate warning: render target period is set to 0ms, "
                     << "therefore not regulated by a framerate. Returning 0.";
        return 0;
    }
    return 1000.0/vtkInteractorStyle->m_targetMS;
}

void
VTKViewer::setTargetFrameRate(const double& fps)
{
    auto vtkInteractorStyle = std::dynamic_pointer_cast<VTKInteractorStyle>(m_interactorStyle);

    if(fps < 0)
    {
        LOG(WARNING) << "VTKViewer::setTargetFrameRate error: framerate must be positive, "
                     << "or equal to 0 to render as fast as possible.";
        return;
    }
    if(fps == 0)
    {
        vtkInteractorStyle->m_targetMS = 0;
        return;
    }
    vtkInteractorStyle->m_targetMS = 1000.0/fps;
    std::cout << "Target framerate: " << fps << " (" << vtkInteractorStyle->m_targetMS << " ms)"<< std::endl;
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
} // imstk
