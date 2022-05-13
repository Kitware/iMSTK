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

class vtkRenderWindow;
class vtkCallbackCommand;
class vtkInteractorStyle;

namespace imstk
{
///
/// \class AbstractVTKViewer
///
/// \brief Subclasses viewer for the VTK rendering back-end
/// \todo: Implement pausing/resuming
///
class AbstractVTKViewer : public Viewer
{
public:
    enum class VTKLoggerMode
    {
        SHOW,
        MUTE,
        WRITE
    };

protected:
    AbstractVTKViewer(std::string name);
public:
    ~AbstractVTKViewer() override = default;

    ///
    /// \brief Get the current renderer mode
    ///
    Renderer::Mode getRenderingMode() const override;

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
    /// \brief Set whether to vertical sync. Sync framerate to
    /// the refresh of the monitor
    ///
    void setUseVsync(const bool useVsync) override;

    ///
    /// \brief Set the coloring of the screen background
    /// If 'gradientBackground' is false or not supplied color1 will fill the entire background
    ///
    virtual void setBackgroundColors(const Color color1, const Color color2 = Color(0.0, 0.0, 0.0),
                                     const bool gradientBackground = false) override;

    ///
    /// \brief Processes VTK events, includes OS events
    ///
    void processEvents() override;

    ///
    /// \brief Set the logger mode
    ///
    void setVtkLoggerMode(VTKLoggerMode loggerMode);

protected:
    bool initModule() override;

    void uninitModule() override;

    vtkSmartPointer<vtkRenderWindow>    m_vtkRenderWindow;
    vtkSmartPointer<vtkInteractorStyle> m_vtkInteractorStyle;
    vtkSmartPointer<vtkCallbackCommand> exitCallback;
    bool m_useVsync = false;
};
} // namespace imstk