/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkVTKRenderer.h"
#include "imstkCamera.h"
#include "imstkDirectionalLight.h"
#include "imstkPointLight.h"
#include "imstkSpotLight.h"
#include "imstkLogger.h"
#include "imstkScene.h"
#include "imstkSceneObject.h"
#include "imstkTextureManager.h"
#include "imstkVisualModel.h"
#include "imstkVTKSurfaceMeshRenderDelegate.h"

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
#include <vtkSSAOPass.h>
#include <vtkProperty.h>

namespace imstk
{
VTKRenderer::VTKRenderer(std::shared_ptr<Scene> scene, const bool enableVR) :
    m_scene(scene),
    m_textureManager(std::make_shared<TextureManager<VTKTextureDelegate>>()),
    m_ssaoPass(vtkSmartPointer<vtkSSAOPass>::New()),
    m_renderStepsPass(vtkSmartPointer<vtkRenderStepsPass>::New())
{
    // create m_vtkRenderer depending on enableVR
    m_VrEnabled = enableVR;
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
}

void
VTKRenderer::initialize()
{
    // Process all the changes initially (add all the delegates)
    sceneModifed(nullptr);
    this->updateRenderDelegates();

    // Lights and light actors
    for (const auto& light : m_scene->getLights())
    {
        std::string name = light->getTypeName();
        if (name == DirectionalLight::getStaticTypeName())
        {
            auto lightVtk = vtkSmartPointer<vtkLight>::New();
            lightVtk->SetPositional(false);
            const Color& color = light->getColor();
            lightVtk->SetColor(color.r, color.g, color.b);
            lightVtk->SetIntensity(light->getIntensity());
            lightVtk->SetFocalPoint(light->getFocalPoint().data());
            lightVtk->SetPosition(0.0, 0.0, 0.0);
            lightVtk->SetAttenuationValues(light->getAttenuationValues().data());

            m_vtkLights.push_back(VtkLightPair(light, lightVtk));
        }
        else if (name == SpotLight::getStaticTypeName())
        {
            auto lightVtk = vtkSmartPointer<vtkLight>::New();
            lightVtk->SetPositional(true);
            const Color& color = light->getColor();
            lightVtk->SetColor(color.r, color.g, color.b);
            lightVtk->SetIntensity(light->getIntensity());
            lightVtk->SetFocalPoint(light->getFocalPoint().data());

            auto spotLight = std::dynamic_pointer_cast<SpotLight>(light);
            lightVtk->SetPosition(spotLight->getPosition().data());
            lightVtk->SetConeAngle(spotLight->getSpotAngle());
            lightVtk->SetAttenuationValues(light->getAttenuationValues().data());

            m_vtkLights.push_back(VtkLightPair(light, lightVtk));

            auto lightActorSpot = vtkSmartPointer<vtkLightActor>::New();
            lightActorSpot->SetLight(lightVtk);
            m_debugVtkActors.push_back(lightActorSpot);
        }
        else if (name == PointLight::getStaticTypeName())
        {
            auto lightVtk = vtkSmartPointer<vtkLight>::New();
            lightVtk->SetPositional(true);
            const Color& color = light->getColor();
            lightVtk->SetColor(color.r, color.g, color.b);
            lightVtk->SetIntensity(light->getIntensity());
            lightVtk->SetFocalPoint(light->getFocalPoint().data());
            lightVtk->SetAttenuationValues(light->getAttenuationValues().data());

            auto pointLight = std::dynamic_pointer_cast<PointLight>(light);
            lightVtk->SetPosition(pointLight->getPosition().data());

            m_vtkLights.push_back(VtkLightPair(light, lightVtk));

            auto lightActorPoint = vtkSmartPointer<vtkLightActor>::New();
            lightActorPoint->SetLight(lightVtk);
            m_debugVtkActors.push_back(lightActorPoint);
        }
        else
        {
            LOG(WARNING) << "Light type undefined!";
        }
    }

    for (const auto& light : m_vtkLights)
    {
        m_vtkRenderer->AddLight(light.second);
    }

    // Camera and camera actor
    if (!m_VrEnabled)
    {
        m_camera = vtkSmartPointer<vtkCamera>::New();
    }
    else
    {
        m_camera = vtkSmartPointer<vtkOpenVRCamera>::New();
    }

    updateCamera();
    vtkNew<vtkCameraActor> camActor;
    camActor->SetCamera(m_camera);
    m_debugVtkActors.push_back(camActor);

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

    this->setConfig(this->m_config);

    m_isInitialized = true;
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
                m_vtkRenderer->AddLight(light.second);
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
                m_vtkRenderer->AddLight(light.second);
            }
        }
        else if (m_currentMode == Mode::Debug)
        {
            this->removeActors(m_debugVtkActors);
        }
    }

    // Reset the camera
    m_camera = vtkSmartPointer<vtkCamera>::New();

    Renderer::setMode(mode, enableVR);
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
    getVtkRenderer()->SetActiveCamera(m_camera);

    // As long as we don't have a VR camera apply the camera view
    if (auto vtkVRCam = vtkOpenVRCamera::SafeDownCast(m_camera))
    {
        // Copy back the view values from the VR cam to hmd_view in ours
        vtkMatrix4x4* finalView = m_camera->GetModelViewTransformMatrix();
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                cam->m_hmdView(i, j) = finalView->GetElement(j, i);
            }
        }
    }
    else
    {
        // Update the camera to obtain corrected view/proj matrices
        cam->update();

        // Get the view matrix
        const Mat4d& invView = cam->getInvView();

        const double eyePos[3]  = { invView(0, 3), invView(1, 3), invView(2, 3) };
        const double forward[3] = { invView(0, 2), invView(1, 2), invView(2, 2) };
        const double up[3]      = { invView(0, 1), invView(1, 1), invView(2, 1) };

        m_camera->SetPosition(eyePos);
        m_camera->SetFocalPoint(eyePos[0] - forward[0], eyePos[1] - forward[1], eyePos[2] - forward[2]);
        m_camera->SetViewUp(up[0], up[1], up[2]);
        m_camera->SetViewAngle(cam->getFieldOfView());
    }
    m_camera->SetClippingRange(cam->getNearZ(), cam->getFarZ());

    // Copy the projection back to the camera
    auto projVtk = vtkSmartPointer<vtkMatrix4x4>::Take(m_camera->GetProjectionTransformMatrix(m_vtkRenderer));
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            cam->m_proj(i, j) = projVtk->GetElement(j, i);
        }
    }
}

void
VTKRenderer::updateRenderDelegates()
{
    // Update their render delegates
    for (auto delegate : m_renderDelegates)
    {
        delegate.second->update();
    }

    // Update all lights (we don't use render delegates for these as there usually aren't
    // all that many lights)
    for (auto light : m_vtkLights)
    {
        std::shared_ptr<Light>    lightImstk = light.first;
        vtkSmartPointer<vtkLight> lightVtk   = light.second;
        std::string               lightName  = lightImstk->getTypeName();
        if (lightName == "DirectionalLight")
        {
            const Color& color = lightImstk->getColor();
            lightVtk->SetColor(color.r, color.g, color.b);
            lightVtk->SetIntensity(lightImstk->getIntensity());
            lightVtk->SetFocalPoint(lightImstk->getFocalPoint().data());
            lightVtk->SetPosition(0.0, 0.0, 0.0);
            lightVtk->SetAttenuationValues(lightImstk->getAttenuationValues().data());
        }
        else if (lightName == "SpotLight")
        {
            const Color& color = lightImstk->getColor();
            lightVtk->SetColor(color.r, color.g, color.b);
            lightVtk->SetIntensity(lightImstk->getIntensity());
            lightVtk->SetFocalPoint(lightImstk->getFocalPoint().data());

            auto spotLight = std::dynamic_pointer_cast<SpotLight>(lightImstk);
            lightVtk->SetPosition(spotLight->getPosition().data());
            lightVtk->SetConeAngle(spotLight->getSpotAngle());
            lightVtk->SetAttenuationValues(lightImstk->getAttenuationValues().data());
        }
        else if (lightName == "PointLight")
        {
            const Color& color = lightImstk->getColor();
            lightVtk->SetColor(color.r, color.g, color.b);
            lightVtk->SetIntensity(lightImstk->getIntensity());
            lightVtk->SetFocalPoint(lightImstk->getFocalPoint().data());
            lightVtk->SetAttenuationValues(lightImstk->getAttenuationValues().data());

            auto pointLight = std::dynamic_pointer_cast<PointLight>(lightImstk);
            lightVtk->SetPosition(pointLight->getPosition().data());
        }
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
VTKRenderer::addEntity(std::shared_ptr<Entity> entity)
{
    m_renderedObjects.insert(entity);
    m_renderedVisualModels[entity] = std::unordered_set<std::shared_ptr<VisualModel>>();
    entityModified(entity);
    // Observe changes on this SceneObject
    connect<Event>(entity, &Entity::modified, this, &VTKRenderer::entityModified);
}

void
VTKRenderer::addVisualModel(std::shared_ptr<Entity> sceneObject, std::shared_ptr<VisualModel> visualModel)
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
VTKRenderer::removeVisualModel(std::shared_ptr<Entity> sceneObject, std::shared_ptr<VisualModel> visualModel)
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

std::unordered_set<std::shared_ptr<Entity>>::iterator
VTKRenderer::removeEntity(std::shared_ptr<Entity> entity)
{
    auto iter = m_renderedObjects.erase(m_renderedObjects.find(entity));

    // Remove every delegate associated and remove its actors from the scene
    for (auto visualModel : entity->getComponents<VisualModel>())
    {
        removeVisualModel(entity, visualModel);
    }

    m_renderedVisualModels.erase(entity);

    // Stop observing changes on the scene object
    disconnect(entity, this, &SceneObject::modified);
    return iter;
}

void
VTKRenderer::sceneModifed(Event* imstkNotUsed(e))
{
    // If the SceneObject is in the scene but not being rendered
    for (auto ent : m_scene->getSceneObjects())
    {
        if (m_renderedObjects.count(ent) == 0)
        {
            addEntity(ent);
        }
    }
    // If the SceneObject is being rendered but not in the scene
    for (auto i = m_renderedObjects.begin(); i != m_renderedObjects.end(); i++)
    {
        auto sos = m_scene->getSceneObjects();
        if (sos.find(*i) == sos.end())
        {
            i = removeEntity(*i);
        }
    }
}

void
VTKRenderer::entityModified(Event* e)
{
    Entity* sceneObject = static_cast<Entity*>(e->m_sender);
    if (sceneObject != nullptr)
    {
        // Note: All other solutions lead to some ugly variant, I went with this one
        auto iter = std::find_if(m_renderedObjects.begin(), m_renderedObjects.end(),
            [sceneObject](const std::shared_ptr<Entity>& i) { return i.get() == sceneObject; });
        if (iter != m_renderedObjects.end())
        {
            entityModified(*iter);
        }
    }
}

void
VTKRenderer::entityModified(std::shared_ptr<Entity> sceneObject)
{
    // Only diff a sceneObject being rendered
    if (m_renderedObjects.count(sceneObject) == 0 || m_renderedVisualModels.count(sceneObject) == 0)
    {
        return;
    }

    // Now check for added/removed VisualModels

    // If the VisualModel of the SceneObject is in the SceneObject but not being rendered
    const auto& visualModels = sceneObject->getComponents<VisualModel>();
    for (auto visualModel : visualModels)
    {
        if (m_renderedVisualModels[sceneObject].count(visualModel) == 0)
        {
            addVisualModel(sceneObject, visualModel);
        }
    }
    // If the VisualModel of the SceneObject is being rendered but not part of the SceneObject anymore
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
VTKRenderer::setConfig(std::shared_ptr<RendererConfig> config)
{
    m_config = config;

    // update SSAO if enabled
    if (m_config->m_ssaoConfig.m_enableSSAO)
    {
        m_config->m_ssaoConfig.m_SSAOBlur ? m_ssaoPass->BlurOn() : m_ssaoPass->BlurOff(); // Blur on/off
        m_ssaoPass->SetRadius(m_config->m_ssaoConfig.m_SSAORadius);                       // comparison radius
        m_ssaoPass->SetBias(m_config->m_ssaoConfig.m_SSAOBias);                           // comparison bias
        m_ssaoPass->SetKernelSize(m_config->m_ssaoConfig.m_KernelSize);                   // number of samples used

        m_ssaoPass->SetDelegatePass(m_renderStepsPass);
        m_vtkRenderer->SetPass(m_ssaoPass);
    }
    else
    {
        m_vtkRenderer->SetPass(nullptr);
    }

    // update background colors
    m_vtkRenderer->SetBackground(m_config->m_BGColor1.r, m_config->m_BGColor1.g, m_config->m_BGColor1.b);
    m_vtkRenderer->SetBackground2(m_config->m_BGColor2.r, m_config->m_BGColor2.g, m_config->m_BGColor2.b);
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
} // namespace imstk