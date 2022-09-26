/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkViewer.h"

#include <vtkSmartPointer.h>

class vtkRenderWindow;
class vtkCallbackCommand;
class vtkInteractorStyle;

namespace imstk
{
class VTKRenderer;

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

    ///
    /// \brief Retrieve the renderer associated with the current scene
    ///
    std::shared_ptr<VTKRenderer> getActiveVtkRenderer() const;

protected:
    bool initModule() override;

    void uninitModule() override;

    vtkSmartPointer<vtkRenderWindow>    m_vtkRenderWindow;
    vtkSmartPointer<vtkInteractorStyle> m_vtkInteractorStyle;
    vtkSmartPointer<vtkCallbackCommand> exitCallback;
    bool m_useVsync = false;
};
} // namespace imstk