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

#ifndef imstkVTKRenderer_h
#define imstkVTKRenderer_h

#include <memory>
#include <vector>

#include "vtkSmartPointer.h"
#include "vtkRenderer.h"
#include "vtkProp.h"
#include "vtkCamera.h"
#include "vtkLight.h"

namespace imstk
{

class Scene;
class Camera;
class VTKRenderDelegate;

///
/// \class Renderer
///
/// \brief
///
class VTKRenderer
{
public:
    ///
    /// \brief Enumerations for the render mode
    ///
    enum Mode
    {
        EMPTY,
        DEBUG,
        SIMULATION
    };

    ///
    /// \brief Constructor
    ///
    VTKRenderer(std::shared_ptr<Scene> scene);

    ///
    /// \brief Default destructor
    ///
    ~VTKRenderer() = default;

    ///
    /// \brief Set/Get the rendering mode which defined the
    /// visibility of the renderer actors and the default camera
    ///
    void setMode(Mode mode);
    const Mode& getMode();

    ///
    /// \brief
    ///
    void updateSceneCamera(std::shared_ptr<Camera> imstkCam);

    ///
    /// \brief
    ///
    void updateRenderDelegates();

    ///
    /// \brief
    ///
    vtkSmartPointer<vtkRenderer> getVtkRenderer() const;

protected:
    ///
    /// \brief
    ///
    void removeActors(const std::vector<vtkSmartPointer<vtkProp>>& actorList);

    ///
    /// \brief
    ///
    void addActors(const std::vector<vtkSmartPointer<vtkProp>>& actorList);

    vtkSmartPointer<vtkRenderer> m_vtkRenderer = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkCamera> m_defaultVtkCamera;
    vtkSmartPointer<vtkCamera> m_sceneVtkCamera;
    std::vector<vtkSmartPointer<vtkLight>> m_vtkLights;
    std::vector<vtkSmartPointer<vtkProp>> m_objectVtkActors;
    std::vector<vtkSmartPointer<vtkProp>> m_debugVtkActors;

    std::vector<std::shared_ptr<VTKRenderDelegate>> m_renderDelegates;

    Mode m_currentMode = Mode::EMPTY;
};
}

#endif // ifndef imstkRenderer_h
