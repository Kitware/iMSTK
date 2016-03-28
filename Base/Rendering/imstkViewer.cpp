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

#include "vtkRenderer.h"
#include "vtkPlaneSource.h"
#include "vtkPolyDataMapper.h"

#include "g3log/g3log.hpp"

namespace imstk {
void
Viewer::initRenderer()
{
    // Check current Scene
    if (m_currentScene == nullptr)
    {
        LOG(WARNING) << "No scene specified, nothing to render.";
        return;
    }

    // Check RenderWindow
    if (m_renderWindow == nullptr)
    {
        LOG(INFO) << "No render window specified, creating default render window.";
        m_renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    }

    // Create renderer
    vtkSmartPointer<vtkRenderer> renderer =
        vtkSmartPointer<vtkRenderer>::New();
    m_renderWindow->AddRenderer(renderer);

    // WIP : Create renderdelegates with actors
    // based on the current scene objects.
    vtkSmartPointer<vtkPlaneSource> planeSource =
        vtkSmartPointer<vtkPlaneSource>::New();
    planeSource->SetCenter(1.0, 0.0, 0.0);
    planeSource->SetNormal(1.0, 0.0, 1.0);
    planeSource->Update();
    vtkPolyData *plane                        = planeSource->GetOutput();
    vtkSmartPointer<vtkPolyDataMapper> mapper =
        vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(plane);
    vtkSmartPointer<vtkActor> actor =
        vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    renderer->AddActor(actor);
}

vtkSmartPointer<vtkRenderWindow>
Viewer::getRenderWindow() const
{
    return m_renderWindow;
}

void
Viewer::setRenderWindow(vtkSmartPointer<vtkRenderWindow>renWin)
{
    m_renderWindow = renWin;
}

std::shared_ptr<Scene>
Viewer::getCurrentScene() const
{
    return m_currentScene;
}

void
Viewer::setCurrentScene(std::shared_ptr<Scene>scene)
{
    m_currentScene = scene;
}

void
Viewer::initModule()
{
    LOG(DEBUG) << m_name << " : init";
    this->initRenderer();
    m_renderWindow->SetWindowName(m_name.data());
    m_renderWindow->Start();
}

void
Viewer::cleanUpModule()
{
    LOG(DEBUG) << m_name << " : cleanUp";
    m_renderWindow->Finalize();
}

void
Viewer::runModule()
{
    LOG(DEBUG) << m_name << " : running";
    m_renderWindow->Render();
}
}
