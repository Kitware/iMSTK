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

#ifndef METALSHADER_H
#define METALSHADER_H

// SimMedTK includes
#include "smShader/smShader.h"

namespace smtk{
namespace Event{

    class smEvent;
}
}

/// \brief metal shader look. It is mainly used for tool rendering but utilized for high specularity rendering for tissues.
class smMetalShader: public smShader
{
public:
    /// \brief constructor that get vertex and fragment shader file name.
    smMetalShader(const smString &p_verteShaderFileName = "shaders/VertexBumpMap1.cg",
                  const smString &p_fragmentFileName = "shaders/FragmentBumpMap1.cg");

    /// \brief attach mesh to the shader
    void attachMesh(std::shared_ptr<smMesh> p_mesh, smChar *p_bump,
                    smChar *p_decal, smChar *p_specular,
                    smChar *p_OCC, smChar *p_disp);

    void attachMesh(std::shared_ptr<smMesh> p_mesh, smChar *p_bump, smChar *p_decal, smChar *p_specular, smChar *p_OCC, smChar *p_disp, smChar *p_alphaMap);

    /// \brief emtpy implementation of draw routine. needs to overwritten to enable real-time code changes
    void draw(const smDrawParam &p_param);

    /// \brief initializaiton of bindings
    virtual void initDraw(const smDrawParam &p_param);

    /// \brief uniforms are set in the predraw
    virtual void predraw(std::shared_ptr<smMesh> mesh);

    virtual void predraw(std::shared_ptr<smSurfaceMesh> mesh);

    /// \brief handle keyboard event
    void handleEvent(std::shared_ptr<smtk::Event::smEvent> p_event) override;

    /// \brief any disable and enable shader options need to be here
    virtual void switchEnable();

    virtual void switchDisable();

public:
    smGLInt lightPower; // light power
    smGLInt roughness; // oughness of surface
    smGLInt tangent; // tangent vectors attribute GL binding
    smGLInt specularPower; // specular power
    smGLFloat specularPowerValue; // specular power value, coefficient used in shader
    smInt attrib; // for future use
    smGLInt alphaMapGain; // alpha map gain
    smGLFloat alphaMapGainValue; // alpha map gain coefficient
    smGLInt canGetShadowUniform; // to enable/disable to shadow on particular object.
};

/// \brief MetalShader variantion with shadow feature
class MetalShaderShadow: public smMetalShader
{
public:
    /// \brief MetalShader v
    MetalShaderShadow(const smString &p_vertexShaderFileName,const smString &p_fragmentShaderFileName);

    /// \brief intialization routine
    virtual void initDraw(const smDrawParam &p_param);

    /// \brief unifom binding called before object is rendered
    virtual void predraw(std::shared_ptr<smMesh> mesh);

    virtual void predraw(std::shared_ptr<smSurfaceMesh> mesh);

private:
    /// \brief for debugging purposes
    smGLInt shadowMapUniform;
    smGLInt canGetShadowUniform;
};

/// \brief another variation of metalshader with differen shadow mapping technique
class MetalShaderSoftShadow: public smMetalShader
{
public:
    /// \brief constrcutore with vertex and fragment shader
    MetalShaderSoftShadow();

    /// \brief initialization routine
    virtual void initDraw(const smDrawParam &p_param);

    /// \brief pre rendering routine before attached object is rendered
    virtual void predraw(std::shared_ptr<smMesh> p_mesh);

    virtual void predraw(std::shared_ptr<smSurfaceMesh> mesh);

private:
    smGLInt shadowMapUniform;
};

#endif
