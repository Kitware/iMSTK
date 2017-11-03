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

#ifndef imstkVTKViewer_h
#define imstkVTKViewer_h

#include <memory>
#include <unordered_map>

#include "g3log/g3log.hpp"

#include "imstkScene.h"
#include "imstkVTKRenderer.h"
#include "imstkVTKInteractorStyle.h"
#include "imstkVTKScreenCaptureUtility.h"
#include "imstkViewer.h"
#include "imstkVTKRenderDelegate.h"
#include "vtkSmartPointer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"

//Screenshot utility
#include "imstkVTKScreenCaptureUtility.h"

namespace imstk
{
class SimulationManager;

///
/// \class Viewer
///
/// \brief Viewer
///
class VTKViewer : public Viewer
{
public:
    ///
    /// \brief Constructor
    ///
    VTKViewer(SimulationManager* manager = nullptr)
    {
        m_interactorStyle = std::make_shared<VTKInteractorStyle>();
        auto vtkInteractorStyle = std::dynamic_pointer_cast<VTKInteractorStyle>(m_interactorStyle);
        vtkInteractorStyle->m_simManager = manager;
        m_vtkRenderWindow->SetInteractor(m_vtkRenderWindow->MakeRenderWindowInteractor());
        m_vtkRenderWindow->GetInteractor()->SetInteractorStyle( vtkInteractorStyle.get() );
        m_vtkRenderWindow->SetSize(1000,800);
        m_screenCapturer = std::make_shared<VTKScreenCaptureUtility>(m_vtkRenderWindow);
    }

    ///
    /// \brief Destructor
    ///
    virtual void setRenderingMode(Renderer::Mode mode);

    ///
    /// \brief Destructor
    ///
    ~VTKViewer() = default;

    ///
    /// \brief Set scene to be rendered
    ///
    virtual void setActiveScene(std::shared_ptr<Scene>scene);

    ///
    /// \brief Get the current renderer mode
    ///
    virtual const Renderer::Mode getRenderingMode();

    ///
    /// \brief Start rendering
    ///
    virtual void startRenderingLoop();

    ///
    /// \brief Terminate rendering
    ///
    virtual void endRenderingLoop();

    ///
    /// \brief Get pointer to the vtkRenderWindow rendering
    ///
    vtkSmartPointer<vtkRenderWindow>getVtkRenderWindow() const;

    ///
    /// \brief Get the target FPS for rendering
    ///
    double getTargetFrameRate() const;

    ///
    /// \brief Set the target FPS for rendering
    ///
    void setTargetFrameRate(const double& fps);

    ///
    /// \brief Access screen shot utility
    ///
    std::shared_ptr<VTKScreenCaptureUtility> getScreenCaptureUtility() const;

    ///
    /// \brief Set the coloring of the screen background
    /// If 'gradientBackground' is false or not supplied color1 will fill the entire background
    ///
    virtual void setBackgroundColors(const Vec3d color1, const Vec3d color2 = Vec3d::Zero(), const bool gradientBackground = false);

protected:

    vtkSmartPointer<vtkRenderWindow> m_vtkRenderWindow = vtkSmartPointer<vtkRenderWindow>::New();
};
} // imstk

#endif // ifndef imstkViewer_h
