// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#ifndef VTKVIEWER_H
#define VTKVIEWER_H

#include "Core/ViewerBase.h"

class VtkViewer : public ViewerBase
{

public:
    VtkViewer();
    ~VtkViewer();

    VtkViewer(const VtkViewer& other) = delete;
    VtkViewer& operator=(const VtkViewer& other) = delete;

public:
    ///
    /// \brief Execute when exit viewer
    ///
    void exitViewer() override {}

    ///
    /// \brief Add text for display
    ///
    void addText(std::string /*tag*/) override {}

    ///
    /// \brief Update text
    ///
    void updateText(std::string /*tag*/, std::string /*string*/) override {}

    ///
    /// \brief Update text
    ///
    void updateText(int /*handle*/, std::string /*string*/) override {}

    ///
    /// \brief Enable/disable VSync
    ///
    void setVSync(bool /*sync*/) override {}

    ///
    /// \brief Execute this module
    ///
    void exec() override;

    ///
    /// \brief Run when frame begins
    ///
    void beginFrame() override {}

    ///
    /// \brief Run when frame ends
    ///
    void endFrame() override {}

    ///
    /// \brief Verify that the rendering pipeline initializes properly
    ///
    bool isValid();

protected:
    ///
    /// \brief Renders the render operation to screen
    ///
    void renderToScreen(const RenderOperation &/*rop*/) override {}

    ///
    /// \brief Renders the render operation to an FBO
    ///
    void renderToFBO(const RenderOperation &/*rop*/) override {}

    ///
    /// \brief Initializes rendering system (e.g., OpenGL) capabilities and flags
    ///
    void initRenderingCapabilities() override {}

    ///
    /// \brief Initilizes the rendering system (e.g., OpenGL) context, and window containing it
    void initRenderingContext() override {}

    ///
    /// \brief Cleans up after initGLContext()
    ///
    void destroyRenderingContext() override {}

    ///
    /// \brief Initializes FBOs, textures, shaders and VAOs
    ///
    void initResources() override;

    ///
    /// \brief Processes viewerRenderDetail options
    ///
    void processViewerOptions() override {}

    ///
    /// \brief Process window events and render as the major part of an event loop
    ///
    void processWindowEvents() override {}

    ///
    /// \brief Set the color and other viewer defaults
    ///
    void setToDefaults() override {}

    ///
    /// \brief render depth texture for debugging
    ///
    void renderTextureOnView() override {}

    ///
    /// \brief Render scene
    ///
    void render() override;

private:
    class VtkRenderer;
    std::unique_ptr<VtkRenderer> renderer;
};


#endif // VTKVIEWER_H
