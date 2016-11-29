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

#ifndef imsktVTKViewer_h
#define imstkVTKViewer_h

#include <memory>
#include <unordered_map>

#include "imstkScene.h"
#include "imstkVTKRenderer.h"
#include "imstkVTKInteractorStyle.h"

#include "vtkSmartPointer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"

namespace imstk
{

class SimulationManager;

///
/// \class Viewer
///
/// \brief Viewer
///
class VTKViewer
{
public:
    ///
    /// \brief
    ///
    VTKViewer(SimulationManager* manager = nullptr)
    {
        m_interactorStyle->setSimulationManager(manager);
        m_vtkRenderWindow->SetInteractor(m_vtkRenderWindow->MakeRenderWindowInteractor());
        m_vtkRenderWindow->GetInteractor()->SetInteractorStyle( m_interactorStyle );
        m_vtkRenderWindow->SetSize(1000,800);
    }

    ///
    /// \brief
    ///
    ~VTKViewer() = default;

    ///
    /// \brief
    ///
    std::shared_ptr<Scene> getCurrentScene() const;

    ///
    /// \brief
    ///
    void setCurrentScene(std::shared_ptr<Scene>scene);

    ///
    /// \brief
    ///
    std::shared_ptr<VTKRenderer> getCurrentRenderer() const;

    ///
    /// \brief
    ///
    void setRenderingMode(VTKRenderer::Mode mode);

    ///
    /// \brief
    ///
    void startRenderingLoop();

    ///
    /// \brief
    ///
    void endRenderingLoop();

    ///
    /// \brief
    ///
    vtkSmartPointer<vtkRenderWindow>getVtkRenderWindow() const;

    ///
    /// \brief
    ///
    const bool& isRendering() const;

    ///
    /// \brief
    ///
    double getTargetFrameRate() const;

    ///
    /// \brief
    ///
    void setTargetFrameRate(const double &fps);

protected:

    vtkSmartPointer<vtkRenderWindow> m_vtkRenderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    vtkSmartPointer<VTKInteractorStyle> m_interactorStyle = vtkSmartPointer<VTKInteractorStyle>::New();
    std::shared_ptr<Scene> m_currentScene;
    std::unordered_map<std::shared_ptr<Scene>, std::shared_ptr<VTKRenderer>> m_rendererMap;
    bool m_running = false;
};

} // imstk

#endif // ifndef imstkViewer_h
