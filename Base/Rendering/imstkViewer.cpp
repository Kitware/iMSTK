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
#include "vtkLight.h"
#include "vtkLightActor.h"
#include "vtkCamera.h"
#include "vtkCameraActor.h"
#include "vtkAxesActor.h"
#include "vtkInteractorStyleTrackballCamera.h"

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

    // Create and add renderer
    auto renderer = vtkSmartPointer<vtkRenderer>::New();
    //renderer->UseShadowsOn();
    m_renderWindow->AddRenderer(renderer);

    // Create and add actors
    for (const auto& obj : m_currentScene->getSceneObjects())
    {
        auto geom     = obj->getVisualGeometry();
        auto delegate = RenderDelegate::make_delegate(geom);
        renderer->AddActor(delegate->getVtkActor());
    }

    // Add lights
    for (const auto& light : m_currentScene->getLights())
    {
        renderer->AddLight( light->getVtkLight() );
        if( light->isPositional() )
        {
            auto lightActor = vtkSmartPointer<vtkLightActor>::New();
            lightActor->SetLight( light->getVtkLight() );
            renderer->AddActor( lightActor );
        }
    }

    // Add camera
    //auto camActor = vtkSmartPointer<vtkCameraActor>::New();
    //camActor->SetCamera(  m_currentScene->getCamera()->getVtkCamera() );
    //renderer->AddActor( camActor );
    //renderer->GetActiveCamera()->SetFocalPoint(0,0,0);
    renderer->SetActiveCamera( m_currentScene->getCamera()->getVtkCamera() );
    renderer->ResetCameraClippingRange();


    // Global Axis
    auto axes = vtkSmartPointer<vtkAxesActor>::New();
    renderer->AddActor(axes);

    // Customize background colors
    renderer->SetBackground(0.66,0.66,0.66);
    renderer->SetBackground2(157.0/255.0*0.66,186/255.0*0.66,192.0/255.0*0.66);
    renderer->GradientBackgroundOn();
}

void
Viewer::startRenderingLoop()
{
    auto style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    m_interactor->SetInteractorStyle( style );
    m_interactor->SetRenderWindow( m_renderWindow );
    m_interactor->Start();
}

void
Viewer::endRenderingLoop()
{
    m_interactor->TerminateApp();
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
}
