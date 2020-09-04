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

#include <vtkSmartPointer.h>
#include <unordered_map>

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
class Camera;
class VTKRenderDelegate;

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
    vtkSmartPointer<vtkRenderer> m_vtkRenderer;

    // Camera
    vtkSmartPointer<vtkCamera> m_Camera;

    // lights
    std::vector<vtkSmartPointer<vtkLight>> m_vtkLights;

    // Props to be rendered
    std::vector<vtkSmartPointer<vtkProp>> m_objectVtkActors;
    std::vector<vtkSmartPointer<vtkProp>> m_debugVtkActors;
    vtkSmartPointer<vtkAxesActor> m_AxesActor;

    // Render delegates for the props
    std::vector<std::shared_ptr<VTKRenderDelegate>> m_renderDelegates;
    std::vector<std::shared_ptr<VTKRenderDelegate>> m_debugRenderDelegates;

    // imstk scene
    std::shared_ptr<Scene> m_scene;

    TextureManager<VTKTextureDelegate> m_textureManager;

    vtkSmartPointer<vtkChartXY>      m_timeTableChart;
    vtkSmartPointer<vtkContextActor> m_timeTableChartActor;
    vtkSmartPointer<vtkTable> m_timeTable;
    vtkPlotBar* m_timeTablePlot;
    int m_timeTableIter = 0;
};
}
