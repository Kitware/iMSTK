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

#include "imstkViewer.h"

#include <vtkSmartPointer.h>

class vtkInteractorStyle;
class vtkRenderWindow;
class vtkCallbackCommand;
class vtkObject;

namespace imstk
{
class VTKScreenCaptureUtility;
class VTKTextStatusManager;

///
/// \class AbstractVTKViewer
///
/// \brief Subclasses viewer for the VTK rendering back-end
/// \todo: Implement pausing/resuming
///
class AbstractVTKViewer : public Viewer
{
protected:
    AbstractVTKViewer(std::string name);
public:
    virtual ~AbstractVTKViewer() override = default;

public:
    ///
    /// \brief Terminate rendering
    ///
    virtual void stopThread() override;

    ///
    /// \brief Pause is reimplemented for VTK viewers
    ///
    void pause(bool sync = false) override;

    ///
    /// \brief Resume is reimplemented for VTK viewers
    ///
    void resume(bool sync = false) override;

public:
    ///
    /// \brief Get the current renderer mode
    ///
    virtual Renderer::Mode getRenderingMode() const override;

    ///
    /// \brief Get pointer to the vtkRenderWindow rendering
    ///
    vtkSmartPointer<vtkRenderWindow> getVtkRenderWindow() const { return m_vtkRenderWindow; }

    ///
    /// \brief Set the render window size
    ///
    void setSize(const int width, const int height) override;

    ///
    /// \brief Set the render window title
    ///
    void setWindowTitle(const std::string& title) override;

    ///
    /// \brief Set the coloring of the screen background
    /// If 'gradientBackground' is false or not supplied color1 will fill the entire background
    ///
    virtual void setBackgroundColors(const Vec3d color1, const Vec3d color2 = Vec3d::Zero(),
                                     const bool gradientBackground = false) override;

    ///
    /// \brief Returns the vtk interactor style
    ///
    std::shared_ptr<vtkInteractorStyle> getVtkInteractorStyle() const { return m_vtkInteractorStyle; }

protected:
    ///
    /// \brief Callback for when the viewer is disabled
    ///
    static void viewerDisabled(vtkObject* sender, unsigned long eventId, void* clientData, void* callData);
    ///
    /// \brief Callback for when the viewer is enabled
    ///
    static void viewerEnabled(vtkObject* sender, unsigned long eventId, void* clientData, void* callData);

protected:
    vtkSmartPointer<vtkRenderWindow>    m_vtkRenderWindow;
    std::shared_ptr<vtkInteractorStyle> m_vtkInteractorStyle;
    vtkSmartPointer<vtkCallbackCommand> viewerDisabledCallback;
    vtkSmartPointer<vtkCallbackCommand> viewerEnabledCallback;
};
} // imstk
