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
#include "Core/MakeUnique.h"

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

class VtkViewer : public ViewerBase
{

public:
    VtkViewer() : renderer(make_unique<VtkRenderer>(safeDownCast<VtkViewer>()))
    {
    }

    ~VtkViewer();

    bool operator==(const VtkViewer& other) const;
    VtkViewer(const VtkViewer& other);
    VtkViewer& operator=(const VtkViewer& other);

public:
    ///
    /// \brief Execute when exit viewer
    ///
    virtual void exitViewer(){}

    ///
    /// \brief add text for display
    ///
    virtual void addText(std::string /*tag*/){}

    ///
    /// \brief update text
    ///
    virtual void updateText(std::string /*tag*/, std::string /*string*/){}

    ///
    /// \brief update text
    ///
    virtual void updateText(int /*handle*/, std::string /*string*/){}

    ///
    /// \brief enable/disable VSync
    ///
    virtual void setVSync(bool /*sync*/){}

protected:
    ///
    /// \brief Renders the render operation to screen
    ///
    virtual void renderToScreen(const RenderOperation &/*rop*/){}

    ///
    /// \brief Renders the render operation to an FBO
    ///
    virtual void renderToFBO(const RenderOperation &/*rop*/){}

    ///
    /// \brief Initializes rendering system (e.g., OpenGL) capabilities and flags
    ///
    virtual void initRenderingCapabilities(){}

    ///
    /// \brief Initilizes the rendering system (e.g., OpenGL) context, and window containing it
    virtual void initRenderingContext();

    ///
    /// \brief Cleans up after initGLContext()
    ///
    virtual void destroyRenderingContext(){}

    ///
    /// \brief Initializes FBOs, textures, shaders and VAOs
    ///
    virtual void initResources(){}

    ///
    /// \brief Processes viewerRenderDetail options
    ///
    virtual void processViewerOptions(){}

    ///
    /// \brief Process window events and render as the major part of an event loop
    ///
    virtual void processWindowEvents(){}

    ///
    /// \brief Set the color and other viewer defaults
    ///
    virtual void setToDefaults(){}

    ///
    /// \brief render depth texture for debugging
    ///
    virtual void renderTextureOnView(){}

    virtual void exec();


    void render();

private:
    class VtkRenderer;
    std::unique_ptr<VtkRenderer> renderer;
};


#endif // VTKVIEWER_H
