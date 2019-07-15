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

#include "imstkVTKRenderer.h"

#include "imstkScene.h"
#include "imstkCamera.h"
#include "imstkVTKRenderDelegate.h"
#include "imstkVTKSurfaceMeshRenderDelegate.h"

#include "vtkLightActor.h"
#include "vtkCameraActor.h"
#include "vtkAxesActor.h"
#include "vtkCullerCollection.h"

#include "g3log/g3log.hpp"

namespace imstk
{
VTKRenderer::VTKRenderer(std::shared_ptr<Scene> scene, const bool enableVR)
{
    m_scene = scene;

    // create m_vtkRenderer depending on enableVR
    if (!enableVR)
    {
        m_vtkRenderer = vtkSmartPointer<vtkRenderer>::New();
    }
#ifdef iMSTK_ENABLE_VR
    else
    {
        m_vtkRenderer = vtkSmartPointer<vtkOpenVRRenderer>::New();
    }

#endif
    this->updateRenderDelegates();

    // Initialize textures for surface mesh render delegates
    for (const auto& renderDelegate : m_renderDelegates)
    {
        auto smRenderDelegate = std::dynamic_pointer_cast<VTKSurfaceMeshRenderDelegate>(renderDelegate);
        if (smRenderDelegate)
        {
            smRenderDelegate->initializeTextures(m_textureManager);
        }
    }

    // Lights and light actors
    for (const auto& light : scene->getLights())
    {
        // Create lights specified in the scene
        switch (light->getType())
        {
        case imstk::LightType::DIRECTIONAL_LIGHT:
        {
            auto m_vtkLight = vtkSmartPointer<vtkLight>::New();
            m_vtkLight->SetPositional(false);
            auto color = light->getColor();
            m_vtkLight->SetColor(color.r, color.g, color.b);
            m_vtkLight->SetIntensity(light->getIntensity());
            m_vtkLight->SetFocalPoint(light->getFocalPoint().data());

            m_vtkLights.push_back(m_vtkLight);
        }
        break;

        case imstk::LightType::SPOT_LIGHT:
        {
            auto m_vtkLight = vtkSmartPointer<vtkLight>::New();
            m_vtkLight->SetPositional(true);
            auto color = light->getColor();
            m_vtkLight->SetColor(color.r, color.g, color.b);
            m_vtkLight->SetIntensity(light->getIntensity());
            m_vtkLight->SetFocalPoint(light->getFocalPoint().data());

            auto spotLight = std::dynamic_pointer_cast<SpotLight>(light);
            m_vtkLight->SetConeAngle(spotLight->getConeAngle());
            m_vtkLight->SetPosition(spotLight->getPosition().data());
            m_vtkLight->SetConeAngle(spotLight->getSpotAngle());

            m_vtkLights.push_back(m_vtkLight);

            auto lightActorSpot = vtkSmartPointer<vtkLightActor>::New();
            lightActorSpot->SetLight(m_vtkLight);
            m_debugVtkActors.push_back(lightActorSpot);
        }
        break;

        case imstk::LightType::POINT_LIGHT:
        {
            auto m_vtkLight = vtkSmartPointer<vtkLight>::New();
            m_vtkLight->SetPositional(true);
            auto color = light->getColor();
            m_vtkLight->SetColor(color.r, color.g, color.b);
            m_vtkLight->SetIntensity(light->getIntensity());
            m_vtkLight->SetFocalPoint(light->getFocalPoint().data());

            auto pointLight = std::dynamic_pointer_cast<PointLight>(light);
            m_vtkLight->SetConeAngle(pointLight->getConeAngle());
            m_vtkLight->SetPosition(pointLight->getPosition().data());

            m_vtkLights.push_back(m_vtkLight);

            auto lightActorPoint = vtkSmartPointer<vtkLightActor>::New();
            lightActorPoint->SetLight(m_vtkLight);
            m_debugVtkActors.push_back(lightActorPoint);
        }
        break;

        default:

            LOG(WARNING) << "Light type undefined!";
        }
    }

    // Global Axis
    auto axes = vtkSmartPointer<vtkAxesActor>::New();
    axes->SetShaftType(vtkAxesActor::CYLINDER_SHAFT);
    axes->SetAxisLabels(false);
    axes->SetTotalLength(40, 40, 40);
    //m_debugVtkActors.push_back(axes);

    // Camera and camera actor
    if (!enableVR)
    {
        m_sceneVtkCamera = vtkSmartPointer<vtkCamera>::New();
    }
#ifdef iMSTK_ENABLE_VR
    else
    {
        m_sceneVtkCamera = vtkSmartPointer<vtkOpenVRCamera>::New();
    }
#endif
    this->updateSceneCamera(scene->getCamera());
    auto camActor = vtkSmartPointer<vtkCameraActor>::New();
    camActor->SetCamera(m_sceneVtkCamera);
    m_debugVtkActors.push_back(camActor);

    // Debug camera
    if (!enableVR)
    {
        m_defaultVtkCamera = m_vtkRenderer->GetActiveCamera();
        m_vtkRenderer->ResetCamera();
    }
#ifdef iMSTK_ENABLE_VR
    else
    {
        // in VR mode, we keep m_defaultVtkCamera as the HMD camera
        // => Do not change the active Camera
        m_defaultVtkCamera = vtkSmartPointer<vtkOpenVRCamera>::New();
        m_vtkRenderer->SetActiveCamera(m_defaultVtkCamera);
    }

#endif
    ///TODO : based on scene properties
    // Customize background colors

    //m_vtkRenderer->SetBackground(0.66,0.66,0.66);
    //m_vtkRenderer->SetBackground2(0.406, 0.481, 0.497);

    m_vtkRenderer->SetBackground(0.3285, 0.3285, 0.6525);
    m_vtkRenderer->SetBackground2(0.13836, 0.13836, 0.274);

    m_vtkRenderer->GradientBackgroundOn();

    ///TODO : only on deformable objects with UseBounds (need vtk update)
    // Remove culling
    if (auto culler = m_vtkRenderer->GetCullers()->GetLastItem())
    {
        m_vtkRenderer->RemoveCuller(culler);
    }

    // set camera pos
#ifdef iMSTK_ENABLE_VR
    if (enableVR)
    {
        m_camPos.resize(2);

        // save debug camera m_camPos[0]
        m_defaultVtkCamera->GetPosition(m_camPos[0].Position);
        m_defaultVtkCamera->GetDirectionOfProjection(m_camPos[0].ViewDirection);

        m_camPos[0].PhysicalViewUp[0] = 0.0;
        m_camPos[0].PhysicalViewUp[1] = 1.0;
        m_camPos[0].PhysicalViewUp[2] = 0.0;

        m_camPos[0].PhysicalViewDirection[0] = 1.0;
        m_camPos[0].PhysicalViewDirection[1] = 0.0;
        m_camPos[0].PhysicalViewDirection[2] = 0.0;

        m_camPos[0].Distance = 10; // room scale factor

        m_camPos[0].Translation[0] = 0.0;
        m_camPos[0].Translation[1] = 0.0;
        m_camPos[0].Translation[2] = 0.0;

        // save scene camera m_camPos[1]
        m_sceneVtkCamera->GetPosition(m_camPos[1].Position);
        m_sceneVtkCamera->GetDirectionOfProjection(m_camPos[1].ViewDirection);

        m_camPos[1].PhysicalViewUp[0] = 0.0;
        m_camPos[1].PhysicalViewUp[1] = 1.0;
        m_camPos[1].PhysicalViewUp[2] = 0.0;

        m_camPos[1].PhysicalViewDirection[0] = 1.0;
        m_camPos[1].PhysicalViewDirection[1] = 0.0;
        m_camPos[1].PhysicalViewDirection[2] = 0.0;

        m_camPos[1].Distance = 1.0; // room scale factor of 1 meter

        m_camPos[1].Translation[0] = 0.0;
        m_camPos[1].Translation[1] = 0.0;
        m_camPos[1].Translation[2] = 0.0;
    }
#endif
}

vtkSmartPointer<vtkRenderer>
VTKRenderer::getVtkRenderer() const
{
    return m_vtkRenderer;
}

void
VTKRenderer::setMode(const Renderer::Mode mode, const bool enableVR)
{
#ifdef iMSTK_ENABLE_VR
    vtkOpenVRRenderWindow* renWin = nullptr;
    if (enableVR)
    {
        renWin = vtkOpenVRRenderWindow::SafeDownCast(m_vtkRenderer->GetRenderWindow());
        if (!renWin || !m_vtkRenderer)
        {
            return;
        }
    }
#endif
    if (mode == Mode::EMPTY && m_currentMode != Mode::EMPTY)
    {
        this->removeActors(m_objectVtkActors);
        m_vtkRenderer->RemoveAllLights();

        if (m_currentMode == Mode::DEBUG)
        {
            this->removeActors(m_debugVtkActors);
        }
        // do not change active camera in VR
        if (!enableVR)
        {
            m_vtkRenderer->SetActiveCamera(m_defaultVtkCamera);
        }
    }
    else if (mode == Mode::DEBUG && m_currentMode != Mode::DEBUG)
    {
        this->addActors(m_debugVtkActors);

        if (m_currentMode == Mode::EMPTY)
        {
            this->addActors(m_objectVtkActors);
            for (const auto& light : m_vtkLights)
            {
                m_vtkRenderer->AddLight(light);
            }
        }
        // do not change active camera in VR
        if (!enableVR)
        {
            m_vtkRenderer->SetActiveCamera(m_defaultVtkCamera);
            m_vtkRenderer->ResetCamera();
        }
#ifdef iMSTK_ENABLE_VR
        else // go to debug position camera
        {    // apply debug camera
            m_camPos[0].Apply(static_cast<vtkOpenVRCamera*>(m_vtkRenderer->GetActiveCamera()), renWin);
            m_vtkRenderer->ResetCameraClippingRange();
        }
#endif
    }
    else if (mode == Mode::SIMULATION && m_currentMode != Mode::SIMULATION)
    {
        if (m_currentMode == Mode::EMPTY)
        {
            this->addActors(m_objectVtkActors);
            for (const auto& light : m_vtkLights)
            {
                m_vtkRenderer->AddLight(light);
            }
        }
        else if (m_currentMode == Mode::DEBUG)
        {
            this->removeActors(m_debugVtkActors);

#ifdef iMSTK_ENABLE_VR
            // save last debug position camera
            if (enableVR)
            {
                m_camPos[0].Set(
                    static_cast<vtkOpenVRCamera*>(m_vtkRenderer->GetActiveCamera()),
                    vtkOpenVRRenderWindow::SafeDownCast(renWin));
            }
#endif
        }
        // do not change active camera in VR
        if (!enableVR)
        {
            m_vtkRenderer->SetActiveCamera(m_sceneVtkCamera);
        }
#ifdef iMSTK_ENABLE_VR
        else // go to Predefined sceneVtkCamera position
        {    // apply scene camera
            m_camPos[1].Apply(static_cast<vtkOpenVRCamera*>(m_vtkRenderer->GetActiveCamera()), renWin);
        }
#endif
        m_vtkRenderer->ResetCameraClippingRange();
    }

    Renderer::setMode(mode, enableVR);
}

void
VTKRenderer::updateSceneCamera(std::shared_ptr<Camera> imstkCam)
{
    // Get imstk Camera info
    auto p = imstkCam->getPosition();
    auto f = imstkCam->getFocalPoint();
    auto v = imstkCam->getViewUp();

    // Update vtk Camera
    m_sceneVtkCamera->SetPosition(p[0], p[1], p[2]);
    m_sceneVtkCamera->SetFocalPoint(f[0], f[1], f[2]);
    m_sceneVtkCamera->SetViewUp(v[0], v[1], v[2]);
    m_sceneVtkCamera->SetViewAngle(imstkCam->getFieldOfView());
}

void
VTKRenderer::updateRenderDelegates()
{
    // Object actors
    for (const auto& obj : m_scene->getSceneObjects())
    {
        for (auto visualModel : obj->getVisualModels())
        {
            auto geom = visualModel->getGeometry();
            if (visualModel && !visualModel->isRenderDelegateCreated())
            {
                auto delegate = VTKRenderDelegate::makeDelegate(visualModel);
                if (delegate == nullptr)
                {
                    LOG(WARNING) << "Renderer::Renderer error: Could not create render delegate for '"
                                 << obj->getName() << "'.";
                    continue;
                }

                m_renderDelegates.push_back(delegate);
                m_objectVtkActors.push_back(delegate->getVtkActor());
                m_vtkRenderer->AddActor(delegate->getVtkActor());
                visualModel->m_renderDelegateCreated = true;
            }
        }
    }

    // Debug render actors
    for (const auto& geom : m_scene->getDebugRenderObjects())
    {
        if (geom && !geom->m_renderDelegateCreated)
        {
            auto delegate = VTKRenderDelegate::makeDebugDelegate(geom);
            if (delegate == nullptr)
            {
                LOG(WARNING) << "Renderer::Renderer error: Could not create render delegate for '"
                             << geom->getName() << "'.";
                continue;
            }

            m_debugRenderDelegates.push_back(delegate);
            m_objectVtkActors.push_back(delegate->getVtkActor());
            m_vtkRenderer->AddActor(delegate->getVtkActor());
            geom->m_renderDelegateCreated = true;
        }
    }

    for (auto delegate : m_renderDelegates)
    {
        delegate->update();
    }
}

void
VTKRenderer::removeActors(const std::vector<vtkSmartPointer<vtkProp>>& actorList)
{
    for (const auto& actor : actorList)
    {
        m_vtkRenderer->RemoveActor(actor);
    }
}

void
VTKRenderer::addActors(const std::vector<vtkSmartPointer<vtkProp>>& actorList)
{
    for (const auto& actor : actorList)
    {
        m_vtkRenderer->AddActor(actor);
    }
}

void
VTKRenderer::updateBackground(const Vec3d backgroundOne, const Vec3d backgroundTwo /*= Vec3d::Zero()*/, const bool gradientBackground /*= false*/)
{
    m_vtkRenderer->SetBackground(backgroundOne.x(), backgroundOne.y(), backgroundOne.z());
    if (gradientBackground)
    {
        m_vtkRenderer->SetBackground2(backgroundTwo.x(), backgroundTwo.y(), backgroundTwo.z());
        m_vtkRenderer->GradientBackgroundOn();
    }
    else
    {
        m_vtkRenderer->GradientBackgroundOff();
    }
}
} // imstk
