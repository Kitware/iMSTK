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

#pragma once

#include "imstkRenderer.h"
#include "imstkTextureManager.h"
#include "imstkVTKTextureDelegate.h"
#include "imstkEventObject.h"

#include <vtkSmartPointer.h>
#include <unordered_map>
#include <unordered_set>

class vtkAxesActor;
class vtkCamera;
class vtkChartXY;
class vtkContextActor;
class vtkLight;
class vtkOpenVRCamera;
class vtkPlotBar;
class vtkProp;
class vtkRenderer;
class vtkTable;

namespace imstk
{
class Scene;
class SceneObject;
class Camera;
class VTKRenderDelegate;
class VisualModel;

///
/// \class VTKRenderer
///
/// \brief Wraps a vtkRenderer
///
class VTKRenderer : public Renderer
{
public:
    VTKRenderer(std::shared_ptr<Scene> scene, const bool enableVR);
    virtual ~VTKRenderer() override = default;

public:
    ///
    /// \brief Set the rendering mode to display debug actors or not
    ///
    void setMode(const Mode mode, const bool enableVR) override;

    ///
    /// \brief Change the debug axes length
    ///
    void setAxesLength(const double x, const double y, const double z);
    void setAxesLength(const Vec3d& len);

    ///
    /// \brief Get the debug axes length
    ///
    Vec3d getAxesLength();

    ///
    /// \brief Change the visibility of the debug axes
    ///
    void setAxesVisibility(const bool visible);

    ///
    /// \brief Returns whether the debug axes is visible or not
    ///
    bool getAxesVisibility() const;

    ///
    /// \brief Sets the benchmarking table using unordered_map
    ///
    void setTimeTable(const std::unordered_map<std::string, double>& timeTable);

    ///
    /// \brief Set the visibility of the benchmark graph
    ///
    void setTimeTableVisibility(const bool visible);

    ///
    /// \brief Get the visibility of the benchmark graph
    ///
    bool getTimeTableVisibility() const;

    ///
    /// \brief Updates the camera
    ///
    void updateCamera();

    ///
    /// \brief Updates the render delegates
    ///
    void updateRenderDelegates();

    ///
    /// \brief Get the render delegates
    ///
    const std::vector<std::shared_ptr<VTKRenderDelegate>>& getDebugRenderDelegates() const { return m_debugRenderDelegates; }

    ///
    /// \brief Returns VTK renderer
    ///
    vtkSmartPointer<vtkRenderer> getVtkRenderer() const { return m_vtkRenderer; }

    ///
    /// \brief Update background colors
    ///
    void updateBackground(const Vec3d color1, const Vec3d color2 = Vec3d::Zero(), const bool gradientBackground = false);

protected:
    ///
    /// \brief Remove actors (also called props) from the scene
    ///
    void removeActors(const std::vector<vtkSmartPointer<vtkProp>>& actorList);

    ///
    /// \brief Add actors (also called props) from the scene
    ///
    void addActors(const std::vector<vtkSmartPointer<vtkProp>>& actorList);

protected:
    ///
    /// \brief Adds a SceneObject to be rendered
    ///
    void addSceneObject(std::shared_ptr<SceneObject> sceneObject);

    ///
    /// \brief Removes a SceneObject to be rendered
    ///
    std::unordered_set<std::shared_ptr<SceneObject>>::iterator removeSceneObject(std::shared_ptr<SceneObject> sceneObject);

    ///
    /// \brief Callback for when the scene this renderer renders is modified
    /// This involves adding/removing scene objects to render lists
    ///
    void sceneModifed(Event* e);

protected:
    ///
    /// \brief Add a VisualModel to be rendered, creates a delegate for it
    ///
    void addVisualModel(std::shared_ptr<SceneObject> sceneObject, std::shared_ptr<VisualModel> visualModel);

    ///
    /// \brief Remove a VisualModel from rendering
    ///
    std::unordered_set<std::shared_ptr<VisualModel>>::iterator removeVisualModel(std::shared_ptr<SceneObject> sceneObject, std::shared_ptr<VisualModel> visualModel);

    ///
    /// \brief Callback for when a SceneObject is modified
    /// This involves adding/removing visual models to render lists
    ///
    void sceneObjectModified(Event* e);

    ///
    /// \brief Function call for proccessing diffs on a SceneObject
    ///
    void sceneObjectModified(std::shared_ptr<SceneObject> sceneObject);

protected:
    vtkSmartPointer<vtkRenderer> m_vtkRenderer;

    // Camera
    vtkSmartPointer<vtkCamera> m_Camera;

    // lights
    std::vector<vtkSmartPointer<vtkLight>> m_vtkLights;

    // Props to be rendered
    std::vector<vtkSmartPointer<vtkProp>> m_objectVtkActors;
    std::vector<vtkSmartPointer<vtkProp>> m_debugVtkActors;
    vtkSmartPointer<vtkAxesActor> m_AxesActor;

    // imstk scene
    std::shared_ptr<Scene> m_scene;

    // Rendered Objects, this gives whats currently being rendered
    std::unordered_set<std::shared_ptr<SceneObject>> m_renderedObjects;
    std::unordered_map<std::shared_ptr<SceneObject>, std::unordered_set<std::shared_ptr<VisualModel>>> m_renderedVisualModels;

    // Render Delegates
    std::unordered_map<std::shared_ptr<VisualModel>, std::shared_ptr<VTKRenderDelegate>> m_renderDelegates;
    std::vector<std::shared_ptr<VTKRenderDelegate>> m_debugRenderDelegates;

    // TextureManager is used to share textures among differing delegates
    TextureManager<VTKTextureDelegate> m_textureManager;

    vtkSmartPointer<vtkChartXY>      m_timeTableChart;
    vtkSmartPointer<vtkContextActor> m_timeTableChartActor;
    vtkSmartPointer<vtkTable> m_timeTable;
    vtkPlotBar* m_timeTablePlot;
    int m_timeTableIter = 0;
};
}
