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

#include "imstkRenderer.h"

#include "imstkRenderDelegate.h"

#include "vtkLight.h"
#include "vtkLightActor.h"
#include "vtkCamera.h"
#include "vtkCameraActor.h"
#include "vtkAxesActor.h"

#include "g3log/g3log.hpp"


namespace imstk {
Renderer::Renderer(std::shared_ptr<Scene> scene)
{
    // Object actors
    for ( const auto& obj : scene->getSceneObjects() )
    {
        auto geom     = obj->getVisualGeometry();
        auto delegate = RenderDelegate::make_delegate( geom );
        m_objectVtkActors.push_back( delegate->getVtkActor() );
    }

    // Lights and light actors
    for ( const auto& light : scene->getLights() )
    {
        m_vtkLights.push_back( light->getVtkLight() );
        if( light->isPositional() )
        {
            auto lightActor = vtkSmartPointer<vtkLightActor>::New();
            lightActor->SetLight( light->getVtkLight() );
            m_debugVtkActors.push_back( lightActor );
        }
    }

    // Camera and camera actor
    m_sceneVtkCamera = scene->getCamera()->getVtkCamera();
    auto camActor = vtkSmartPointer<vtkCameraActor>::New();
    camActor->SetCamera(  m_sceneVtkCamera );
    m_debugVtkActors.push_back( camActor );

    // Global Axis
    auto axes = vtkSmartPointer<vtkAxesActor>::New();
    m_debugVtkActors.push_back( axes );

    ///TODO : based on scene properties
    // Customize background colors
    m_vtkRenderer->SetBackground(0.66,0.66,0.66);
    m_vtkRenderer->SetBackground2(157.0/255.0*0.66,186/255.0*0.66,192.0/255.0*0.66);
    m_vtkRenderer->GradientBackgroundOn();
}

vtkSmartPointer<vtkRenderer>
Renderer::getVtkRenderer() const
{
    return m_vtkRenderer;
}

void
Renderer::setup(Mode mode)
{
    if( mode == Mode::EMPTY && m_currentMode != Mode::EMPTY )
    {
        m_vtkRenderer->SetActiveCamera(m_defaultVtkCamera);

        this->removeActors(m_objectVtkActors);
        m_vtkRenderer->RemoveAllLights();

        if( m_currentMode == Mode::DEBUG )
        {
            this->removeActors(m_debugVtkActors);
        }
    }
    else if( mode == Mode::DEBUG && m_currentMode != Mode::DEBUG )
    {
        m_vtkRenderer->SetActiveCamera(m_defaultVtkCamera);

        this->addActors(m_debugVtkActors);

        if( m_currentMode == Mode::EMPTY )
        {
            this->addActors(m_objectVtkActors);
            for ( const auto& light : m_vtkLights )
            {
                m_vtkRenderer->AddLight(light);
            }
        }
    }
    else if ( mode == Mode::SIMULATION && m_currentMode != Mode::SIMULATION )
    {
        m_vtkRenderer->SetActiveCamera(m_sceneVtkCamera);

        if( m_currentMode == Mode::EMPTY )
        {
            this->addActors(m_objectVtkActors);
            for ( const auto& light : m_vtkLights )
            {
                m_vtkRenderer->AddLight(light);
            }
        }
        else if( m_currentMode == Mode::DEBUG )
        {
            this->removeActors(m_debugVtkActors);
        }
    }

    m_currentMode = mode;
}

void
Renderer::removeActors(const std::vector<vtkSmartPointer<vtkProp>>& actorList)
{
    for ( const auto& actor : actorList )
    {
        m_vtkRenderer->RemoveActor(actor);
    }
}

void
Renderer::addActors(const std::vector<vtkSmartPointer<vtkProp>>& actorList)
{
    for ( const auto& actor : actorList )
    {
        m_vtkRenderer->AddActor(actor);
    }
}
}
