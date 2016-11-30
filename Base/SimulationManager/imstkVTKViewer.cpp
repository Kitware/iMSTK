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

#include "g3log/g3log.hpp"

#include "imstkVTKRenderDelegate.h"

namespace imstk
{

std::shared_ptr<Scene>
VTKViewer::getCurrentScene() const
{
    return m_currentScene;
}

void
VTKViewer::setCurrentScene(std::shared_ptr<Scene>scene)
{
    // If already current scene
    if( scene == m_currentScene )
    {
        LOG(WARNING) << scene->getName() << " already is the viewer current scene.";
        return;
    }

    // If the current scene has a renderer, remove it
    if( m_currentScene )
    {
        auto vtkRenderer = this->getCurrentRenderer()->getVtkRenderer();
        if(m_vtkRenderWindow->HasRenderer(vtkRenderer))
        {
            m_vtkRenderWindow->RemoveRenderer(vtkRenderer);
        }
    }

    // Update current scene
    m_currentScene = scene;

    // Create renderer if it doesn't exist
    if (!m_rendererMap.count(m_currentScene))
    {
        m_rendererMap[m_currentScene] = std::make_shared<VTKRenderer>(m_currentScene);
    }

    // Set renderer to renderWindow
    m_vtkRenderWindow->AddRenderer(this->getCurrentRenderer()->getVtkRenderer());

    // Set renderer to interactorStyle
    m_interactorStyle->SetCurrentRenderer(this->getCurrentRenderer()->getVtkRenderer());

    // Set name to renderWindow
    m_vtkRenderWindow->SetWindowName(m_currentScene->getName().data());
}

std::shared_ptr<VTKRenderer>
VTKViewer::getCurrentRenderer() const
{
    return m_rendererMap.at(m_currentScene);
}

void
VTKViewer::setRenderingMode(VTKRenderer::Mode mode)
{
    if( !m_currentScene )
    {
        LOG(WARNING) << "Missing scene, can not set rendering mode.\n"
                     << "Use Viewer::setCurrentScene to setup scene.";
        return;
    }

    // Setup renderer
    this->getCurrentRenderer()->setup(mode);
    if( !m_running )
    {
        return;
    }

    // Render to update displayed actors
    m_vtkRenderWindow->Render();

    // Setup render window
    if (mode == VTKRenderer::Mode::SIMULATION)
    {
        m_interactorStyle->HighlightProp(nullptr);
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
    m_vtkRenderWindow->GetInteractor()->TerminateApp();
}

vtkSmartPointer<vtkRenderWindow>
VTKViewer::getVtkRenderWindow() const
{
    return m_vtkRenderWindow;
}

const bool&
VTKViewer::isRendering() const
{
    return m_running;
}

double
VTKViewer::getTargetFrameRate() const
{
    return m_interactorStyle->getTargetFrameRate();
}

void
VTKViewer::setTargetFrameRate(const double& fps)
{
    m_interactorStyle->setTargetFrameRate(fps);
}

} // imstk
