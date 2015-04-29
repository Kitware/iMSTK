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

// SimMedTK includes
#include "smShader/smShader.h"

namespace smtk{
namespace Event{

    class smEvent;
}
}

/// \brief scene texture shader. This shader works on the scene that is placed on 2D image. It is for image based effects
class smSceneTextureShader: public smShader
{
public:
    /// \brief constructor that receives the vertex and fragment shader file names
    smSceneTextureShader(const smString &p_verteShaderFileName = "shaders/renderSceneVertexShader.glsl",
                         const smString &p_fragmentFileName = "shaders/renderSceneFragShader.glsl");

    /// \brief called during  rendering initialization
    void initDraw(const smDrawParam &p_param);

    /// \brief if object is added to viewer, it is called from viewer. used for debuginng purposes. It is needed to see effect of instant code changes in shader code
    void draw(const smDrawParam &p_param);

    /// \brief pre drawing of the shader. used for binding the uniforms if there are.
    virtual void predraw(std::shared_ptr<smMesh> p_mesh);

    virtual void predraw(std::shared_ptr<smSurfaceMesh> /*mesh*/){};

    /// \brief handle the events
    virtual void handleEvent(std::shared_ptr<smtk::Event::smEvent> p_event) override;

public:
    /// \brief depth  texture GL id that is sent to shader. It stores scene depth values
    smGLInt depthTex;
    /// \brief scene  texture GL id that is sent to shader. It stores scene RGB values
    smGLInt sceneTex;
    /// \brief previous  texture GL id that is sent to shader. It stores previous image RGB values rendered in previous rendering stage in multipass
    smGLInt  prevTex;
};

#endif
