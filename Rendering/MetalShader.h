// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
//
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

#ifndef METALSHADER_H
#define METALSHADER_H

// iMSTK includes
#include "Rendering/Shader.h"

namespace mstk{
namespace Event{

    class Event;
}
}

namespace Core {
    class BaseMesh;
}
class SurfaceMesh;

/// \brief metal shader look. It is mainly used for tool rendering but utilized for high specularity rendering for tissues.
class MetalShader: public Shader
{
public:
    /// \brief constructor that get vertex and fragment shader file name.
    MetalShader(const std::string &p_verteShaderFileName = "shaders/VertexBumpMap1.cg",
                  const std::string &p_fragmentFileName = "shaders/FragmentBumpMap1.cg");

    /// \brief attach mesh to the shader
    void attachMesh(std::shared_ptr<Core::BaseMesh> p_mesh, char *p_bump,
                    char *p_decal, char *p_specular,
                    char *p_OCC, char *p_disp);

    void attachMesh(std::shared_ptr<Core::BaseMesh> p_mesh, char *p_bump, char *p_decal, char *p_specular, char *p_OCC, char *p_disp, char *p_alphaMap);

    /// \brief emtpy implementation of draw routine. needs to overwritten to enable real-time code changes
    void draw() const override;

    /// \brief initializaiton of bindings
    virtual void initDraw() override;

    /// \brief uniforms are set in the predraw
    virtual void predraw(std::shared_ptr<Core::BaseMesh> mesh) override;

    virtual void predraw(std::shared_ptr<SurfaceMesh> mesh) override;

    /// \brief handle keyboard event
    void handleEvent(std::shared_ptr<core::Event> p_event) override;

    /// \brief any disable and enable shader options need to be here
    virtual void switchEnable();

    virtual void switchDisable();

public:
    GLint lightPower; // light power
    GLint roughness; // oughness of surface
    GLint tangent; // tangent vectors attribute GL binding
    GLint specularPower; // specular power
    GLfloat specularPowerValue; // specular power value, coefficient used in shader
    int attrib; // for future use
    GLint alphaMapGain; // alpha map gain
    GLfloat alphaMapGainValue; // alpha map gain coefficient
    GLint canGetShadowUniform; // to enable/disable to shadow on particular object.
};

/// \brief MetalShader variantion with shadow feature
class MetalShaderShadow: public MetalShader
{
public:
    /// \brief MetalShader v
    MetalShaderShadow(const std::string &p_vertexShaderFileName,const std::string &p_fragmentShaderFileName);

    /// \brief intialization routine
    virtual void initDraw() override;

    /// \brief unifom binding called before object is rendered
    virtual void predraw(std::shared_ptr<Core::BaseMesh> mesh) override;

    virtual void predraw(std::shared_ptr<SurfaceMesh> mesh) override;

private:
    /// \brief for debugging purposes
    GLint shadowMapUniform;
    GLint canGetShadowUniform;
};

/// \brief another variation of metalshader with differen shadow mapping technique
class MetalShaderSoftShadow: public MetalShader
{
public:
    /// \brief constrcutore with vertex and fragment shader
    MetalShaderSoftShadow();

    /// \brief initialization routine
    virtual void initDraw() override;

    /// \brief pre rendering routine before attached object is rendered
    virtual void predraw(std::shared_ptr<Core::BaseMesh> p_mesh) override;

    virtual void predraw(std::shared_ptr<SurfaceMesh> mesh) override;

private:
    GLint shadowMapUniform;
};

#endif
