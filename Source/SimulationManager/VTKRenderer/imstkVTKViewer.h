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

#include "imstkVTKScreenCaptureUtility.h"
#include "imstkViewer.h"
#include "imstkVTKRenderDelegate.h"
#include "imstkVTKScreenCaptureUtility.h"

#ifdef iMSTK_ENABLE_VR
#include "imstkOpenVRCommand.h"
#include "vtkOpenVRRenderWindow.h"
#include "vtkOpenVRRenderWindowInteractor.h"
#endif

class vtkCallbackCommand;
class vtkRenderWindow;

namespace imstk
{
class SimulationManager;
class VTKTextStatusManager;
class VTKInteractorStyle;
class Scene;

///
/// \class VTKViewer
///
/// \brief Subclasses viewer for the VTK rendering back-end
///
class VTKViewer : public Viewer
{
public:
    ///
    /// \brief Constructor
    ///
    VTKViewer(SimulationManager* manager = nullptr, bool enableVR = false);

    ///
    /// \brief Destructor
    ///
    virtual void setRenderingMode(const Renderer::Mode mode) override;

    ///
    /// \brief Set scene to be rendered
    ///
    virtual void setActiveScene(const std::shared_ptr<Scene>& scene) override;

    ///
    /// \brief Get the current renderer mode
    ///
    virtual Renderer::Mode getRenderingMode() override;

    ///
    /// \brief Start rendering
    ///
    virtual void startRenderingLoop() override;

    ///
    /// \brief Terminate rendering
    ///
    virtual void endRenderingLoop() override;

    ///
    /// \brief Get pointer to the vtkRenderWindow rendering
    ///
    vtkSmartPointer<vtkRenderWindow> getVtkRenderWindow() const;

    ///
    /// \brief Access screen shot utility
    ///
    std::shared_ptr<VTKScreenCaptureUtility> getScreenCaptureUtility() const;

    ///
    /// \brief Set the coloring of the screen background
    /// If 'gradientBackground' is false or not supplied color1 will fill the entire background
    ///
    virtual void setBackgroundColors(const Vec3d color1, const Vec3d color2 = Vec3d::Zero(),
                                     const bool gradientBackground = false) override;

    ///
    /// \brief set the window title
    ///
    virtual void setWindowTitle(const std::string& title);

    ///
    /// \brief Return the window status handler
    ///
    const std::shared_ptr<VTKTextStatusManager>& getTextStatusManager();

protected:
    static void timerCallback(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData);

protected:
    vtkSmartPointer<vtkRenderWindow>    m_vtkRenderWindow;
    std::shared_ptr<VTKInteractorStyle> m_vtkInteractorStyle;
    bool m_enableVR;
    std::string m_windowName = "imstk";
    vtkSmartPointer<vtkCallbackCommand> timerCallbackCommand;

#ifdef iMSTK_ENABLE_VR
    vtkSmartPointer<OpenVRCommand> m_openVRCommand;
#endif
};
} // imstk
