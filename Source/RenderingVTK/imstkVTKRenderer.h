/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkRenderer.h"
#include "imstkTextureManager.h"
#include "imstkVTKTextureDelegate.h"
#include "imstkEventObject.h"

#include <vtkSmartPointer.h>
#include <unordered_set>

class vtkCamera;
class vtkCameraPass;
class vtkChartXY;
class vtkContextActor;
class vtkLight;
class vtkOpenVRCamera;
class vtkPlotBar;
class vtkProp;
class vtkRenderer;
class vtkRenderStepsPass;
class vtkSSAOPass;
class vtkTable;

namespace imstk
{
class Camera;
class Entity;
class Light;
class Scene;
class VisualModel;
class VTKRenderDelegate;

///
/// \class VTKRenderer
///
/// \brief Wraps a vtkRenderer
///
class VTKRenderer : public Renderer
{
public:
    ///
    /// \brief Scene is tied to this renderer
    ///
    VTKRenderer(std::shared_ptr<Scene> scene, const bool enableVR);
    ~VTKRenderer() override = default;

    ///
    /// \brief Create the rendering delegates and related VTK objects according
    /// to the scene.
    ///
    void initialize() override;

    ///
    /// \brief Set the rendering mode to display debug actors or not
    ///
    void setMode(const Mode mode, const bool enableVR) override;

    ///
    /// \brief Sets the benchmarking table using unordered_map
    ///
    void setTimeTable(const std::unordered_map<std::string, double>& timeTable) override;

    ///
    /// \brief Get/Set the visibility of the benchmark graph
    /// @{
    void setTimeTableVisibility(const bool visible) override;
    bool getTimeTableVisibility() const override;
    /// @}

    ///
    /// \brief Updates the camera
    ///
    void updateCamera();

    ///
    /// \brief Updates the render delegates
    ///
    void updateRenderDelegates();

    ///
    /// \brief Returns VTK renderer
    ///
    vtkSmartPointer<vtkRenderer> getVtkRenderer() const { return m_vtkRenderer; }

    ///
    /// \brief Update background colors
    ///
    void updateBackground(const Vec3d color1, const Vec3d color2 = Vec3d::Zero(), const bool gradientBackground = false);

    void setDebugActorsVisible(const bool debugActorsVisible);
    bool getDebugActorsVisible() const { return m_debugActorsVisible; }

protected:
    ///
    /// \brief Remove actors (also called props) from the scene
    ///
    void removeActors(const std::vector<vtkSmartPointer<vtkProp>>& actorList);

    ///
    /// \brief Add actors (also called props) from the scene
    ///
    void addActors(const std::vector<vtkSmartPointer<vtkProp>>& actorList);

    ///
    /// \brief Apply configuration changes
    ///
    void setConfig(std::shared_ptr<RendererConfig> config) override;

    ///
    /// \brief Adds an entity to be rendered
    ///
    void addEntity(std::shared_ptr<Entity> sceneObject);

    ///
    /// \brief Removes a SceneObject to be rendered
    ///
    std::unordered_set<std::shared_ptr<Entity>>::iterator removeEntity(std::shared_ptr<Entity> sceneObject);

    ///
    /// \brief Callback for when the scene this renderer renders is modified
    /// This involves adding/removing scene objects to render lists
    ///
    void sceneModifed(Event* e);

    ///
    /// \brief Add a VisualModel to be rendered, creates a delegate for it
    ///
    void addVisualModel(std::shared_ptr<Entity> sceneObject, std::shared_ptr<VisualModel> visualModel);

    ///
    /// \brief Remove a VisualModel from rendering
    ///
    std::unordered_set<std::shared_ptr<VisualModel>>::iterator removeVisualModel(std::shared_ptr<Entity> sceneObject, std::shared_ptr<VisualModel> visualModel);

    ///
    /// \brief Callback for when a SceneObject is modified
    /// This involves adding/removing visual models to render lists
    ///
    void entityModified(Event* e);

    ///
    /// \brief Function call for processing diffs on a SceneObject
    ///
    void entityModified(std::shared_ptr<Entity> sceneObject);

    vtkSmartPointer<vtkRenderer> m_vtkRenderer;

    // Camera
    vtkSmartPointer<vtkCamera> m_camera;

    // lights
    using VtkLightPair = std::pair<std::shared_ptr<Light>, vtkSmartPointer<vtkLight>>;
    std::vector<VtkLightPair> m_vtkLights;

    // Props to be rendered
    std::vector<vtkSmartPointer<vtkProp>> m_objectVtkActors;
    std::vector<vtkSmartPointer<vtkProp>> m_debugVtkActors;

    // imstk scene
    std::shared_ptr<Scene> m_scene;

    // Rendered Objects, this gives whats currently being rendered
    std::unordered_set<std::shared_ptr<Entity>> m_renderedObjects;
    std::unordered_map<std::shared_ptr<Entity>, std::unordered_set<std::shared_ptr<VisualModel>>> m_renderedVisualModels;

    // Render Delegates
    std::unordered_map<std::shared_ptr<VisualModel>, std::shared_ptr<VTKRenderDelegate>> m_renderDelegates;

    // TextureManager is used to share textures among differing delegates
    std::shared_ptr<TextureManager<VTKTextureDelegate>> m_textureManager;

    // Performance chart overlay
    vtkSmartPointer<vtkChartXY>      m_timeTableChart;
    vtkSmartPointer<vtkContextActor> m_timeTableChartActor;
    vtkSmartPointer<vtkTable> m_timeTable;
    vtkPlotBar* m_timeTablePlot;
    int m_timeTableIter = 0;

    // SSAO Effect
    vtkSmartPointer<vtkSSAOPass> m_ssaoPass;
    vtkSmartPointer<vtkRenderStepsPass> m_renderStepsPass;

    bool m_debugActorsVisible;
};
} // namespace imstk
