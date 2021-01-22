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
#include "imstkCamera.h"
#include "imstkDebugRenderGeometry.h"
#include "imstkLight.h"
#include "imstkLogger.h"
#include "imstkScene.h"
#include "imstkSceneObject.h"
#include "imstkTextureManager.h"
#include "imstkVisualModel.h"
#include "imstkVTKSurfaceMeshRenderDelegate.h"

#include <vtkAxesActor.h>
#include <vtkCameraActor.h>
#include <vtkCullerCollection.h>
#include <vtkLight.h>
#include <vtkLightActor.h>

#include <vtkAxis.h>
#include <vtkChartXY.h>
#include <vtkContextActor.h>
#include <vtkContextScene.h>
#include <vtkDoubleArray.h>
#include <vtkPlotBar.h>
#include <vtkStringArray.h>
#include <vtkTable.h>
#include <vtkTextProperty.h>

#include <vtkCameraPass.h>
#include <vtkOpenVRCamera.h>
#include <vtkOpenVRRenderer.h>
#include <vtkRenderPassCollection.h>
#include <vtkRenderStepsPass.h>
#include <vtkSequencePass.h>
#include <vtkShadowMapBakerPass.h>
#include <vtkShadowMapPass.h>
#include <vtkSSAOPass.h>
#include <vtkProperty.h>

namespace imstk
{
VTKRenderer::VTKRenderer(std::shared_ptr<Scene> scene, const bool enableVR) :
    m_scene(scene), 
    m_textureManager(std::make_shared<TextureManager<VTKTextureDelegate>>()),
    m_ssaoPass(vtkSmartPointer<vtkSSAOPass>::New()),
    m_renderStepsPass(vtkSmartPointer<vtkRenderStepsPass>::New()),
    m_shadowPass(vtkSmartPointer<vtkShadowMapPass>::New()),
    m_cameraPass(vtkSmartPointer<vtkCameraPass>::New())
{
    // create m_vtkRenderer depending on enableVR
    if (!enableVR)
    {
        m_vtkRenderer = vtkSmartPointer<vtkRenderer>::New();
    }
    else
    {
        m_vtkRenderer = vtkSmartPointer<vtkOpenVRRenderer>::New();
        vtkOpenVRRenderer::SafeDownCast(m_vtkRenderer)->SetAutomaticLightCreation(false);
        vtkOpenVRRenderer::SafeDownCast(m_vtkRenderer)->SetLightFollowCamera(false);
    }

    // Process all the changes initially (add all the delegates)
    sceneModifed(nullptr);
    this->updateRenderDelegates();

    // Lights and light actors
    for (const auto& light : scene->getLights())
    {
        // Create lights specified in the scene
        switch (light->getType())
        {
        case imstk::LightType::Directional:
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

        case imstk::LightType::Spot:
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

        case imstk::LightType::Point:
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

    for (const auto& light : m_vtkLights)
    {
        m_vtkRenderer->AddLight(light);
    }

    // Global Axis
    m_AxesActor = vtkSmartPointer<vtkAxesActor>::New();
    m_AxesActor->SetShaftType(vtkAxesActor::CYLINDER_SHAFT);
    m_AxesActor->SetAxisLabels(true);
    m_debugVtkActors.push_back(m_AxesActor);

    // Camera and camera actor
    if (!enableVR)
    {
        m_Camera = vtkSmartPointer<vtkCamera>::New();
    }
    else
    {
        m_Camera = vtkSmartPointer<vtkOpenVRCamera>::New();
    }

    updateCamera();
    vtkNew<vtkCameraActor> camActor;
    camActor->SetCamera(m_Camera);
    m_debugVtkActors.push_back(camActor);

    ///TODO : based on scene properties
    // Customize background colors

    m_vtkRenderer->SetBackground(m_config->m_BGColor1.r, m_config->m_BGColor1.g, m_config->m_BGColor1.b);
    m_vtkRenderer->SetBackground2(m_config->m_BGColor2.r, m_config->m_BGColor2.g, m_config->m_BGColor2.b);

    m_vtkRenderer->GradientBackgroundOn();

    ///TODO : only on deformable objects with UseBounds (need vtk update)
    // Remove culling
    if (auto culler = m_vtkRenderer->GetCullers()->GetLastItem())
    {
        m_vtkRenderer->RemoveCuller(culler);
    }

    // Observe changes to the scene
    connect<Event>(m_scene, &Scene::modified, this, &VTKRenderer::sceneModifed);

    {
        // Add the benchmarking chart
        m_timeTableChart = vtkSmartPointer<vtkChartXY>::New();
        vtkSmartPointer<vtkContextScene> m_benchmarkChartScene = vtkSmartPointer<vtkContextScene>::New();
        m_timeTableChartActor = vtkSmartPointer<vtkContextActor>::New();
        m_vtkRenderer->AddActor(m_timeTableChartActor);
        m_benchmarkChartScene->SetRenderer(m_vtkRenderer);

        m_timeTableChart->SetAutoSize(true);
        m_timeTableChart->SetSize(vtkRectf(0.0, 0.0, 600.0, 600.0));

        m_benchmarkChartScene->AddItem(m_timeTableChart);
        m_timeTableChartActor->SetScene(m_benchmarkChartScene);
        m_timeTableChartActor->SetVisibility(false);

        m_timeTablePlot = vtkPlotBar::SafeDownCast(m_timeTableChart->AddPlot(vtkChart::BAR));
        m_timeTablePlot->SetColor(0.6, 0.1, 0.1);
        m_timeTablePlot->SetOrientation(vtkPlotBar::HORIZONTAL);
        m_timeTableChart->GetAxis(vtkAxis::BOTTOM)->SetTitle("ms");
        m_timeTableChart->GetAxis(vtkAxis::LEFT)->SetTitle("");
        m_timeTableChart->GetAxis(vtkAxis::LEFT)->GetLabelProperties()->SetVerticalJustification(VTK_TEXT_CENTERED);
        m_timeTableChart->GetAxis(vtkAxis::LEFT)->GetLabelProperties()->SetJustification(VTK_TEXT_RIGHT);

        m_timeTable = vtkSmartPointer<vtkTable>::New();
        vtkSmartPointer<vtkDoubleArray> xIndices = vtkSmartPointer<vtkDoubleArray>::New();
        xIndices->SetName("Indices");
        xIndices->SetNumberOfValues(0);
        vtkSmartPointer<vtkDoubleArray> yElapsedTimes = vtkSmartPointer<vtkDoubleArray>::New();
        yElapsedTimes->SetName("Elapsed Times");
        yElapsedTimes->SetNumberOfValues(0);
        vtkSmartPointer<vtkStringArray> labels = vtkSmartPointer<vtkStringArray>::New();
        labels->SetName("Labels");
        labels->SetNumberOfValues(0);
        m_timeTable->AddColumn(xIndices);
        m_timeTable->AddColumn(yElapsedTimes);
        m_timeTable->AddColumn(labels);
        m_timeTablePlot->SetInputData(m_timeTable, 0, 1);

        m_timeTableChart->GetAxis(vtkAxis::BOTTOM)->GetLabelProperties()->SetColor(1.0, 1.0, 1.0);
        vtkAxis* axisY = m_timeTableChart->GetAxis(vtkAxis::LEFT);
        axisY->GetLabelProperties()->SetColor(1.0, 1.0, 1.0);
        axisY->SetGridVisible(false);
        axisY->SetCustomTickPositions(xIndices, labels);
    }

    // Prepare screen space ambient occlusion effect
    m_ssaoPass->SetDelegatePass(m_renderStepsPass);

    // Prepare shadow pipeline
    vtkNew<vtkSequencePass>         seq;
    vtkNew<vtkRenderPassCollection> passes;
    passes->AddItem(m_shadowPass->GetShadowMapBakerPass());
    passes->AddItem(m_shadowPass);
    seq->SetPasses(passes);
    m_cameraPass->SetDelegatePass(seq);

    updateConfig();
}

void
VTKRenderer::setMode(const Renderer::Mode mode, const bool enableVR)
{
    if (enableVR)
    {
        return;
    }
    if (mode == Mode::Empty && m_currentMode != Mode::Empty)
    {
        this->removeActors(m_objectVtkActors);
        m_vtkRenderer->RemoveAllLights();

        if (m_currentMode == Mode::Debug)
        {
            this->removeActors(m_debugVtkActors);
        }
    }
    else if (mode == Mode::Debug && m_currentMode != Mode::Debug)
    {
        this->addActors(m_debugVtkActors);

        if (m_currentMode == Mode::Empty)
        {
            this->addActors(m_objectVtkActors);
            for (const auto& light : m_vtkLights)
            {
                m_vtkRenderer->AddLight(light);
            }
        }
    }
    else if (mode == Mode::Simulation && m_currentMode != Mode::Simulation)
    {
        if (m_currentMode == Mode::Empty)
        {
            this->addActors(m_objectVtkActors);
            for (const auto& light : m_vtkLights)
            {
                m_vtkRenderer->AddLight(light);
            }
        }
        else if (m_currentMode == Mode::Debug)
        {
            this->removeActors(m_debugVtkActors);
        }
    }

    // Reset the camera
    m_Camera = vtkSmartPointer<vtkCamera>::New();

    Renderer::setMode(mode, enableVR);
}

void
VTKRenderer::setAxesLength(const double x, const double y, const double z)
{
    m_AxesActor->SetTotalLength(x, y, z);
}

void
VTKRenderer::setAxesLength(const Vec3d& len)
{
    m_AxesActor->SetTotalLength(len.x(), len.y(), len.z());
}

Vec3d
VTKRenderer::getAxesLength()
{
    const auto ptr = m_AxesActor->GetTotalLength();
    return Vec3d(ptr[0], ptr[1], ptr[2]);
}

void
VTKRenderer::setAxesVisibility(const bool visible)
{
    m_AxesActor->SetVisibility(visible);
}

bool
VTKRenderer::getAxesVisibility() const
{
    return m_AxesActor->GetVisibility();
}

void
VTKRenderer::setTimeTable(const std::unordered_map<std::string, double>& timeTable)
{
    // Sort by elapsed times
    std::vector<std::pair<std::string, double>> nameToTimesVec(timeTable.begin(), timeTable.end());
    std::sort(nameToTimesVec.begin(), nameToTimesVec.end(),
        [](const std::pair<std::string, double>& a, const std::pair<std::string, double>& b) { return a.second < b.second; });

    // Construct vtkTable from provided data
    vtkSmartPointer<vtkDoubleArray> xIndices      = vtkDoubleArray::SafeDownCast(m_timeTable->GetColumn(0));
    vtkSmartPointer<vtkDoubleArray> yElapsedTimes = vtkDoubleArray::SafeDownCast(m_timeTable->GetColumn(1));
    vtkSmartPointer<vtkStringArray> labels = vtkStringArray::SafeDownCast(m_timeTable->GetColumn(2));

    labels->SetNumberOfValues(nameToTimesVec.size());
    xIndices->SetNumberOfValues(nameToTimesVec.size());
    yElapsedTimes->SetNumberOfValues(nameToTimesVec.size());
    for (size_t i = 0; i < nameToTimesVec.size(); i++)
    {
        labels->SetValue(i, nameToTimesVec[i].first.c_str());
        xIndices->SetValue(i, i + 1);
        yElapsedTimes->SetValue(i, nameToTimesVec[i].second);
    }

    // The range for the x axis is based on history of the elapsed times
    vtkAxis* botAxis = m_timeTableChart->GetAxis(vtkAxis::BOTTOM);

    // Get the previous and current range
    double newMaxElapsed = yElapsedTimes->GetRange()[1];
    yElapsedTimes->Modified();
    double currMaxElapsed = botAxis->GetMaximum();

    // Always respect the max as all information should be shown
    if (newMaxElapsed > currMaxElapsed)
    {
        botAxis->SetRange(0.0, newMaxElapsed);
    }
    // But if current elapsed is less than the existing one we can lag
    else
    {
        // Lag downscaling by 400 iterations
        if (m_timeTableIter % 400 == 0)
        {
            botAxis->SetRange(0.0, newMaxElapsed);
        }
        else
        {
            botAxis->SetRange(0.0, currMaxElapsed);
        }
        m_timeTableIter++;
    }
    botAxis->Modified();

    vtkAxis* leftAxis = m_timeTableChart->GetAxis(vtkAxis::LEFT);
    leftAxis->SetRange(xIndices->GetRange());
    leftAxis->SetCustomTickPositions(xIndices, labels);

    m_timeTable->Modified();
}

void
VTKRenderer::setTimeTableVisibility(const bool visible)
{
    m_timeTableChartActor->SetVisibility(visible);
}

bool
VTKRenderer::getTimeTableVisibility() const
{
    return m_timeTableChartActor->GetVisibility();
}

void
VTKRenderer::updateCamera()
{
    std::shared_ptr<Camera> cam = m_scene->getActiveCamera();
    getVtkRenderer()->SetActiveCamera(m_Camera);

    // Update the camera to obtain corrected view/proj matrices
    cam->update();

    // Get the view matrix
    const Mat4d& invView = cam->getInvView();

    const double eyePos[3]  = { invView(0, 3), invView(1, 3), invView(2, 3) };
    const double forward[3] = { invView(0, 2), invView(1, 2), invView(2, 2) };
    const double up[3]      = { invView(0, 1), invView(1, 1), invView(2, 1) };
    m_Camera->SetPosition(eyePos);
    m_Camera->SetFocalPoint(eyePos[0] - forward[0], eyePos[1] - forward[1], eyePos[2] - forward[2]);
    m_Camera->SetViewUp(up[0], up[1], up[2]);
    m_Camera->SetViewAngle(cam->getFieldOfView());
    m_Camera->SetClippingRange(cam->getNearZ(), cam->getFarZ());
}

void
VTKRenderer::updateRenderDelegates()
{
    // Call visual update on every scene object
    m_scene->updateVisuals();

    // Update their render delegates
    for (auto delegate : m_renderDelegates)
    {
        delegate.second->update();
    }
    for (auto delegate : m_debugRenderDelegates)
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
VTKRenderer::addSceneObject(std::shared_ptr<SceneObject> sceneObject)
{
    m_renderedObjects.insert(sceneObject);
    m_renderedVisualModels[sceneObject] = std::unordered_set<std::shared_ptr<VisualModel>>();
    sceneObjectModified(sceneObject);
    // Observe changes on this SceneObject
    connect<Event>(sceneObject, &SceneObject::modified, this, &VTKRenderer::sceneObjectModified);
}

void
VTKRenderer::addVisualModel(std::shared_ptr<SceneObject> sceneObject, std::shared_ptr<VisualModel> visualModel)
{
    // Create a delegate for the visual m odel
    auto renderDelegate = m_renderDelegates[visualModel] = VTKRenderDelegate::makeDelegate(visualModel);
    if (renderDelegate == nullptr)
    {
        LOG(WARNING) << "error: Could not create render delegate for '"
                     << sceneObject->getName() << "'.";
        return;
    }
    renderDelegate->setTextureManager(m_textureManager);

    m_renderedVisualModels[sceneObject].insert(visualModel);
    m_objectVtkActors.push_back(renderDelegate->getVtkActor());
    m_vtkRenderer->AddActor(renderDelegate->getVtkActor());

    if (auto smRenderDelegate = std::dynamic_pointer_cast<VTKSurfaceMeshRenderDelegate>(renderDelegate))
    {
        smRenderDelegate->initializeTextures();
    }

    visualModel->setRenderDelegateCreated(this, true);
}

std::unordered_set<std::shared_ptr<VisualModel>>::iterator
VTKRenderer::removeVisualModel(std::shared_ptr<SceneObject> sceneObject, std::shared_ptr<VisualModel> visualModel)
{
    auto renderDelegate = m_renderDelegates[visualModel];
    auto iter = std::find(m_objectVtkActors.begin(), m_objectVtkActors.end(), renderDelegate->getVtkActor());
    if (iter != m_objectVtkActors.end())
    {
        m_objectVtkActors.erase(iter);
    }
    m_vtkRenderer->RemoveActor(renderDelegate->getVtkActor());

    m_renderDelegates.erase(visualModel);
    return m_renderedVisualModels[sceneObject].erase(m_renderedVisualModels[sceneObject].find(visualModel));
}

std::unordered_set<std::shared_ptr<SceneObject>>::iterator
VTKRenderer::removeSceneObject(std::shared_ptr<SceneObject> sceneObject)
{
    auto iter = m_renderedObjects.erase(m_renderedObjects.find(sceneObject));

    // Remove every delegate associated and remove its actors from the scene
    for (auto visualModel : sceneObject->getVisualModels())
    {
        removeVisualModel(sceneObject, visualModel);
    }

    m_renderedVisualModels.erase(sceneObject);

    // Stop observing changes on the scene object
    disconnect(sceneObject, this, &SceneObject::modified);
    return iter;
}

void
VTKRenderer::sceneModifed(Event* imstkNotUsed(e))
{
    // If the SceneObject is in the scene but not being rendered
    for (auto sceneObject : m_scene->getSceneObjects())
    {
        if (m_renderedObjects.count(sceneObject) == 0)
        {
            addSceneObject(sceneObject);
        }
    }
    // If the SceneObject is being rendered but not in the scene
    for (auto i = m_renderedObjects.begin(); i != m_renderedObjects.end(); i++)
    {
        if (!m_scene->hasSceneObject(*i))
        {
            i = removeSceneObject(*i);
        }
    }

    // Debug render actors
    for (const auto& dbgVizModel : m_scene->getDebugRenderModels())
    {
        auto geom = std::static_pointer_cast<DebugRenderGeometry>(dbgVizModel->getDebugGeometry());
        if (dbgVizModel && !dbgVizModel->getRenderDelegateCreated(this))
        {
            auto delegate = VTKRenderDelegate::makeDebugDelegate(dbgVizModel);
            if (delegate == nullptr)
            {
                LOG(WARNING) << "error: Could not create render delegate for '"
                             << geom->getName() << "'.";
                continue;
            }

            m_debugRenderDelegates.push_back(delegate);
            m_objectVtkActors.push_back(delegate->getVtkActor());
            m_vtkRenderer->AddActor(delegate->getVtkActor());
            dbgVizModel->setRenderDelegateCreated(this, true);
        }
    }
}

void
VTKRenderer::sceneObjectModified(Event* e)
{
    SceneObject* sceneObject = static_cast<SceneObject*>(e->m_sender);
    if (sceneObject != nullptr)
    {
        // Note: All other solutions lead to some ugly variant, I went with this one
        auto iter = std::find_if(m_renderedObjects.begin(), m_renderedObjects.end(),
            [sceneObject](const std::shared_ptr<SceneObject>& i) { return i.get() == sceneObject; });
        if (iter != m_renderedObjects.end())
        {
            sceneObjectModified(*iter);
        }
    }
}

void
VTKRenderer::sceneObjectModified(std::shared_ptr<SceneObject> sceneObject)
{
    // Only diff a sceneObject being rendered
    if (m_renderedObjects.count(sceneObject) == 0 || m_renderedVisualModels.count(sceneObject) == 0)
    {
        return;
    }

    // Now check for added/removed VisualModels

    // If the VisualModel of the SceneObject is in the SceneObject but not being rendered
    for (auto visualModel : sceneObject->getVisualModels())
    {
        if (m_renderedVisualModels[sceneObject].count(visualModel) == 0)
        {
            addVisualModel(sceneObject, visualModel);
        }
    }
    // If the VisualModel of the SceneObject is being rendered but not part of the SceneObject anymore
    const auto& visualModels = sceneObject->getVisualModels();
    for (auto i = m_renderedVisualModels[sceneObject].begin(); i != m_renderedVisualModels[sceneObject].end(); i++)
    {
        auto iter = std::find(visualModels.begin(), visualModels.end(), *i);
        if (iter == visualModels.end()) // If end, it is not part of the SceneObject anymore
        {
            i = removeVisualModel(sceneObject, *i);
        }
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

void
VTKRenderer::applyConfigChanges(std::shared_ptr<RendererConfig> config)
{
    bool enableSSAO   = m_config->m_ssaoConfig.m_enableSSAO;
    bool enableShadow = m_config->m_shadowConfig.m_enableShadows;

    {
        m_shadowPass->GetShadowMapBakerPass()->SetResolution(config->m_shadowConfig.m_shadowResolution);
        m_shadowPass->GetShadowMapBakerPass()->Modified();
    }

    {
        m_ssaoPass->SetRadius(config->m_ssaoConfig.m_SSAORadius);     // comparison radius
        m_ssaoPass->SetBias(config->m_ssaoConfig.m_SSAOBias);         // comparison bias
        m_ssaoPass->SetKernelSize(config->m_ssaoConfig.m_KernelSize); // number of samples used

        if (config->m_ssaoConfig.m_SSAOBlur)
        {
            m_ssaoPass->BlurOn(); // blur occlusion
        }
        else
        {
            m_ssaoPass->BlurOff(); // do not blur occlusion
        }
    }

    if (enableSSAO && enableShadow)
    {
        m_ssaoPass->SetDelegatePass(m_cameraPass);
        m_vtkRenderer->SetPass(m_ssaoPass);
    }
    else if (enableSSAO)
    {
        m_ssaoPass->SetDelegatePass(m_renderStepsPass);
        m_vtkRenderer->SetPass(m_ssaoPass);
    }
    else if (enableShadow)
    {
        m_vtkRenderer->SetPass(m_cameraPass);
    }
    else
    {
        m_vtkRenderer->SetPass(NULL);
    }
}

void
VTKRenderer::setDebugActorsVisible(const bool debugActorsVisible)
{
    m_debugActorsVisible = debugActorsVisible;
    for (auto debugActors : m_debugVtkActors)
    {
        debugActors->SetVisibility(debugActorsVisible);
    }
}
} // imstk
