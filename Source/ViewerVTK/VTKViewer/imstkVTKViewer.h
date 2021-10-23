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

#include "imstkAbstractVTKViewer.h"

namespace imstk
{
class KeyboardDeviceClient;
class MouseDeviceClient;
class Scene;
class VTKScreenCaptureUtility;
class VTKTextStatusManager;

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

public:
    ///
    /// \brief Set the rendering mode. In debug, debug actors will be shown.
    ///
    void setRenderingMode(const Renderer::Mode mode) override;

    ///
    /// \brief Set scene to be rendered
    ///
    void setActiveScene(std::shared_ptr<Scene> scene) override;

    ///
    /// \brief Set the length of the debug axes
    ///
    void setDebugAxesLength(double x, double y, double z);

    ///
    /// \brief Access screen shot utility
    ///
    std::shared_ptr<VTKScreenCaptureUtility> getScreenCaptureUtility() const;

    ///
    /// \brief Return the window status handler
    ///
    std::shared_ptr<VTKTextStatusManager> getTextStatusManager() const { return m_textStatusManager; }

    ///
    /// \brief Returns the device that emits key events
    ///
    std::shared_ptr<KeyboardDeviceClient> getKeyboardDevice() const;

    ///
    /// \brief Returns the device that emits mouse events
    ///
    std::shared_ptr<MouseDeviceClient> getMouseDevice() const;

protected:
    bool initModule() override;

    void updateModule() override;

protected:
    std::chrono::high_resolution_clock::time_point m_pre;           ///> time point pre-rendering
    std::chrono::high_resolution_clock::time_point m_post;          ///> time point post-rendering
    std::chrono::high_resolution_clock::time_point m_lastFpsUpdate; ///> time point for last framerate display update

    std::shared_ptr<VTKTextStatusManager> m_textStatusManager;      ///> Handle text statuses, including fps status and custom text status

    bool   m_displayFps = false;                                    ///> hide or display framerate
    double m_lastFps;                                               ///> last framerate value used for moving average estimate
};
} // imstk
