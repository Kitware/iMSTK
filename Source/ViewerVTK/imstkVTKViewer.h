/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkAbstractVTKViewer.h"
#include "imstkMacros.h"

namespace imstk
{
class KeyboardDeviceClient;
class MouseDeviceClient;
class Scene;
class VTKScreenCaptureUtility;

///
/// \class VTKViewer
///
/// \brief Subclasses viewer for the VTK rendering back-end
/// Creates vtk renderer for each scene.
///
class VTKViewer : public AbstractVTKViewer
{
public:
    VTKViewer(std::string name = "VTKViewer");
    ~VTKViewer() override      = default;

    IMSTK_TYPE_NAME(VTKViewer)

    ///
    /// \brief Set the rendering mode. In debug, debug actors will be shown.
    ///
    void setRenderingMode(const Renderer::Mode mode) override;

    ///
    /// \brief Set scene to be rendered
    ///
    void setActiveScene(std::shared_ptr<Scene> scene) override;

    ///
    /// \brief Get the amount of info levels
    ///
    const int getInfoLevelCount() const override { return 3; }

    ///
    /// \brief Access screen shot utility
    ///
    std::shared_ptr<VTKScreenCaptureUtility> getScreenCaptureUtility() const;

    ///
    /// \brief Returns the device that emits key events
    ///
    std::shared_ptr<KeyboardDeviceClient> getKeyboardDevice() const override;

    ///
    /// \brief Returns the device that emits mouse events
    ///
    std::shared_ptr<MouseDeviceClient> getMouseDevice() const override;

protected:
    bool initModule() override;

    void updateModule() override;

    std::chrono::high_resolution_clock::time_point m_pre;           ///< time point pre-rendering
    std::chrono::high_resolution_clock::time_point m_post;          ///< time point post-rendering
    std::chrono::high_resolution_clock::time_point m_lastFpsUpdate; ///< time point for last framerate display update

    bool   m_displayFps = false;                                    ///< hide or display framerate
    double m_lastFps;                                               ///< last framerate value used for moving average estimate
};
} // namespace imstk