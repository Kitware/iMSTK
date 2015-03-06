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

#ifndef SCENETEXTURESHADER_H
#define SCENETEXTURESHADER_H

#include "smShader/smShader.h"
#include "smCore/smEvent.h"
#include "smCore/smEventHandler.h"
#include "smCore/smTextureManager.h"
/// \brief scene texture shader. This shader works on the scene that is placed on 2D image. It is for image based effects
class SceneTextureShader: public smShader, public smEventHandler
{
public:
    /// \brief depth  texture GL id that is sent to shader. It stores scene depth values
    smGLInt depthTex;
    /// \brief scene  texture GL id that is sent to shader. It stores scene RGB values
    smGLInt sceneTex;
    /// \brief previous  texture GL id that is sent to shader. It stores previous image RGB values rendered in previous rendering stage in multipass
    smGLInt  prevTex;
    /// \brief constructor that receives the vertex and fragment shader file names
    SceneTextureShader(smChar *p_verteShaderFileName = "shaders/renderSceneVertexShader.glsl",
                       smChar *p_fragmentFileName = "shaders/renderSceneFragShader.glsl");
    /// \brief called during  rendering initialization
    void initDraw(smDrawParam p_param);
    /// \brief if object is added to viewer, it is called from viewer. used for debuginng purposes. It is needed to see effect of instant code changes in shader code
    void draw(smDrawParam p_param);

    /// \brief pre drawing of the shader. used for binding the uniforms if there are.
    virtual void predraw(smMesh *p_mesh);
    /// \brief handle the events
    virtual void handleEvent(smEvent *p_event);
};

#endif
