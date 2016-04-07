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

#include "imstkViewer.h"

#include "vtkInteractorStyleTrackballCamera.h"

#include "g3log/g3log.hpp"

#include "imstkRenderDelegate.h"

namespace imstk {
std::shared_ptr<Scene>
Viewer::getCurrentScene() const
{
    return m_currentScene;
}

void
Viewer::setCurrentScene(std::shared_ptr<Scene>scene)
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
        auto vtkRenderer = m_rendererMap.at(m_currentScene)->getVtkRenderer();
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
        m_rendererMap[m_currentScene] = std::make_shared<Renderer>(m_currentScene);
    }

    // Set renderer to renderWindow
    m_vtkRenderWindow->AddRenderer(m_rendererMap.at(m_currentScene)->getVtkRenderer());
}

void
Viewer::setRenderingMode(Renderer::Mode mode)
{
    if( !m_currentScene )
    {
        LOG(WARNING) << "Missing scene, can not set rendering mode.\n"
                     << "Use Viewer::setCurrentScene to setup scene.";
        return;
    }

    // Setup renderer
    m_rendererMap.at(m_currentScene)->setup(mode);

    // Setup interactor style
    if( mode == Renderer::Mode::DEBUG )
    {
        auto style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
        m_vtkInteractor->SetInteractorStyle( style );
    }
    else if( mode == Renderer::Mode::SIMULATION )
    {
        auto style = vtkSmartPointer<vtkInteractorStyle>::New();
        m_vtkInteractor->SetInteractorStyle( style );
    }
}

void
Viewer::startRenderingLoop()
{
    // Start interaction loop
    m_vtkInteractor->Start();
}

void
Viewer::endRenderingLoop()
{
    m_vtkInteractor->TerminateApp();
}

vtkSmartPointer<vtkRenderWindow>
Viewer::getVtkRenderWindow() const
{
    return m_vtkRenderWindow;
}
}
