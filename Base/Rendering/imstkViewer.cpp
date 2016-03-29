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
#include "vtkCamera.h"
#include "vtkLight.h"
#include "vtkAxesActor.h"

#include "g3log/g3log.hpp"

#include "imstkRenderDelegate.h"

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

    // Create and add renderer
    auto renderer = vtkSmartPointer<vtkRenderer>::New();
    m_renderWindow->AddRenderer(renderer);

    // Create and add actors
    for (const auto& obj : m_currentScene->getSceneObjects())
    {
        auto geom     = obj->getVisualGeometry();
        auto delegate = RenderDelegate::make_delegate(geom);
        renderer->AddActor(delegate->getVtkActor());
    }

    /// WIP : the following
    // Light
    auto light = vtkSmartPointer<vtkLight>::New();
    light->SetLightTypeToSceneLight();
    light->SetPosition(5, 10, 5);
    light->SetFocalPoint(0, 0, 0);

    // light->SetPositional(true);
    // light->SetConeAngle(10);
    renderer->AddLight(light);

    // Camera
    auto camera = renderer->MakeCamera();
    camera->SetPosition(5, 5, 5);
    camera->SetFocalPoint(0, 0, 0);
    renderer->SetActiveCamera(camera);
    renderer->ResetCameraClippingRange();

    // Global Axis
    auto axes = vtkSmartPointer<vtkAxesActor>::New();
    renderer->AddActor(axes);
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
    m_renderWindow->Render();
}
}
