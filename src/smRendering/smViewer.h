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

#ifndef SMVIEWER_H
#define SMVIEWER_H

// 3rd Party iIncludes
#include <SFML/Window.hpp>

// SimMedTK includes
#include "smCore/smViewerBase.h"
#include "smShader.h"
#include "smGLUtils.h"
#include "smVBO.h"
#include "smSimulators/smPBDSceneObject.h"
#include "smCore/smDataStructures.h"
#include "smSimulators/smStylusObject.h"
#include "smFrameBuffer.h"
#include "smCamera.h"

//forward declaration
class smOpenGLWindowStream;

/// \brief Handles all rendering routines.
class smViewer : public smViewerBase
{
public:
    std::unique_ptr<sf::Context> sfmlContext;
    std::unique_ptr<sf::Window> sfmlWindow;

    std::shared_ptr<smOpenGLWindowStream> windowOutput;

    /// \brief default constructor
    smViewer();
    /// \brief for exit viewer
    virtual void exitViewer() override;
    /// \brief add object for rendering
    void addObject(std::shared_ptr<smCoreClass> object);
    /// \brief add text for display
    virtual void addText(smString p_tag) override;
    /// \brief update text
    virtual void updateText(smString p_tag, smString p_string) override;
    virtual void updateText(smInt p_handle, smString p_string) override;
    /// \brief set scene as texture
    void setSceneAsTextureShader(std::shared_ptr<smSceneTextureShader> p_shader);
    /// \brief set the window title
    void setWindowTitle(const smString &str);
    /// \brief enable/disable VSync
    virtual void setVSync(bool sync) override;
    virtual void registerScene(std::shared_ptr<smScene> p_scene, smRenderTargetType p_target, const smString &p_fboName);
    virtual void addFBO(const smString &p_fboName,
                smTexture *p_colorTex, smTexture *p_depthTex,
                smUInt p_width, smUInt p_height);

    smString windowTitle;
    smColor defaultDiffuseColor;
    smColor defaultAmbientColor;
    smColor defaultSpecularColor;

protected:
    virtual void initRenderingCapabilities() override;
    virtual void initResources() override;
    virtual void initRenderingContext() override;
    virtual void destroyRenderingContext() override;
    /// \brief Cleanup function called on exit to ensure resources are cleaned up
    virtual void cleanUp() override;
    /// \brief Renders the internal sceneList
    void renderSceneList();

    virtual void processViewerOptions() override;
    virtual void processWindowEvents() override;
    virtual void renderToScreen(const smRenderOperation &p_rop) override;
    virtual void renderToFBO(const smRenderOperation &p_rop) override;
    virtual void initFboListItems();
    virtual void destroyFboListItems();
    virtual void setToDefaults() override;
    virtual void beginFrame() override;
    virtual void endFrame() override;
    virtual void renderTextureOnView() override;
    /// \brief  event handler
    virtual void handleEvent(std::shared_ptr<smtk::Event::smEvent> p_event) override;
    /// \brief processes an SFML event
    void processSFMLEvents(const sf::Event& p_event);
};

#endif
