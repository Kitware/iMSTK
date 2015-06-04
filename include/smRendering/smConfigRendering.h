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

#ifndef SMCONFIGRENDERING_H
#define SMCONFIGRENDERING_H

// STL includes
#include <cmath>
#include <memory>
#include <vector>

// SimMedTK includes
#include "smCore/smConfig.h"

class smShader;

/// \brief this shows the Vertex Buffer Object Size
///It should be bigger than the total data of vertices and indices of the scene objects.
///the value is in bytes
#define SIMMEDTK_VBOBUFFER_DATASIZE 500000
#define SIMMEDTK_VBOBUFFER_INDEXSIZE    100000
#define SIMMEDTK_VIEWER_MAXLIGHTS   32

#define SIMMEDTK_MAX_TEXTURENBR 36
#define SIMMEDTK_MAX_VBOBUFFERS 10

/// \brief The configuration for VBO
enum smVBOType
{
    SIMMEDTK_VBO_STATIC,
    SIMMEDTK_VBO_NOINDICESCHANGE,
    SIMMEDTK_VBO_DYNAMIC,
};

/// \brief Vertex Buffer Objects Return Types
enum smVBOResult
{
    SIMMEDTK_VBO_OK,
    SIMMEDTK_VBO_NODATAMEMORY,
    SIMMEDTK_VBO_NOINDEXMEMORY,
    SIMMEDTK_VBO_INVALIDOPERATION,
    SIMMEDTK_VBO_BUFFERPOINTERERROR
};

struct smColor
{
    union
    {
        smFloat rgba[4];
        struct
        {
            smFloat r;
            smFloat g;
            smFloat b;
            smFloat a;
        };
    };
    smColor();
    smColor(smFloat r, smFloat g, smFloat b, smFloat a = 1.0);

    /// \brief Dark ratio. the valu is between 0 and 1.0
    void darken(smFloat p_darkFactor);
    /// \brief lighten the color
    void lighten(smFloat p_darkFactor);

    /// \brief returns the color value given with the index
    smFloat operator()(smInt p_i);
    /// \brief setting
    smColor &operator=(const smColor &p_color);
    /// \brief converts to gl color
    smGLFloat* toGLColor();
    const smGLFloat* toGLColor() const;
    /// \brief set RGB color
    void setValue(smFloat p_red, smFloat p_green, smFloat p_blue, smFloat p_alpha);

    static smColor colorWhite;
    static smColor colorBlue;
    static smColor colorGreen;
    static smColor colorRed;
    static smColor colorGray;
    static smColor colorPink;
    static smColor colorYellow;
};


/// \brief renderType. this shows how the render will be done
#define    SIMMEDTK_RENDER_TEXTURE           (1<<1)
#define    SIMMEDTK_RENDER_MATERIALCOLOR     (1<<2)
#define    SIMMEDTK_RENDER_SHADOWS           (1<<3)
#define    SIMMEDTK_RENDER_COLORMAP          (1<<4)
#define    SIMMEDTK_RENDER_WIREFRAME         (1<<5)
#define    SIMMEDTK_RENDER_VERTICES          (1<<6)
#define    SIMMEDTK_RENDER_HIGHLIGHTVERTICES (1<<7)
#define    SIMMEDTK_RENDER_TRANSPARENT       (1<<8)
#define    SIMMEDTK_RENDER_LOCALAXIS         (1<<9)
#define    SIMMEDTK_RENDER_HIGHLIGHT         (1<<10)
#define    SIMMEDTK_RENDER_TETRAS            (1<<11)
#define    SIMMEDTK_RENDER_SURFACE           (1<<12)
#define    SIMMEDTK_RENDER_CUSTOMRENDERONLY  (1<<13)
#define    SIMMEDTK_RENDER_SHADERTEXTURE     (1<<14)
#define    SIMMEDTK_RENDER_FACES             (1<<15)
///Renders Vertex Buffer Objects
#define    SIMMEDTK_RENDER_VBO               (1<<16)
#define    SIMMEDTK_RENDER_NORMALS           (1<<17)
#define    SIMMEDTK_RENDER_NONE              (1<<31)

/// \brief type definitions for variable viewerRenderDetail in smViewer
#define    SIMMEDTK_VIEWERRENDER_GLOBALAXIS                     (1<<1)
#define    SIMMEDTK_VIEWERRENDER_TEXT                           (1<<2)
#define    SIMMEDTK_VIEWERRENDER_FADEBACKGROUND                 (1<<3)
#define    SIMMEDTK_VIEWERRENDER_FADEBACKGROUNDIMAGE            (1<<4)
#define    SIMMEDTK_VIEWERRENDER_VBO_ENABLED                    (1<<5)
#define    SIMMEDTK_VIEWERRENDER_WIREFRAMEALL                   (1<<6)
#define    SIMMEDTK_VIEWERRENDER_TRANSPARENCY                   (1<<7)
#define    SIMMEDTK_VIEWERRENDER_FULLSCREEN                     (1<<8)
#define    SIMMEDTK_VIEWERRENDER_RESTORELASTCAMSETTINGS         (1<<9)
#define    SIMMEDTK_VIEWERRENDER_DISABLE                        (1<<11)
#define    SIMMEDTK_VIEWERRENDER_DYNAMICREFLECTION              (1<<12)




/// \brief viewer detail. legacy code
struct smViewerDetail
{
    smUInt environment;
    smColor backGroundColor;
};

struct smUnifiedId;


/// \brief smRenderDetail has rendering options and features.
///It shows how the mesh should be rendered
struct smRenderDetail
{
public:
    smRenderDetail();
    smRenderDetail(smUInt type) : renderType(type) { normalLength = 1.0; }

    /// \brief attachment of shader
    void addShader(std::shared_ptr<smUnifiedId> p_shaderID);

    /// \brief attachment of VAO
    void addVAO(std::shared_ptr<smUnifiedId> p_shaderID);

    const smColor &getColorDiffuse() const;

    const smColor &getColorAmbient() const;

    const smColor &getColorSpecular() const;

    const smFloat &getShininess() const;

    const smUInt &getRenderType() const;

    const smFloat &getPointSize() const;

    const smFloat &getLineSize() const;

    const smColor &getNormalColor() const;

    const smColor &getHighLightColor() const;

    const smColor &getVertexColor() const;

    const smColor &getShadowColor() const;

    const smBool &getCastShadow() const;

    const smBool &getCanGetShadow() const;

    const smColor &getWireFrameColor() const;

    const smBool &getDebugDraw() const;

    const std::vector<std::shared_ptr<smUnifiedId>> &getShaders() const;

    const std::vector<smBool> &getShaderEnable() const;

    const std::vector<std::shared_ptr<smUnifiedId>> &getVAOs() const;

    const std::vector<smBool> &getVAOEnable() const;

    void setPointSize(const float size);

    void setLineSize(const float size);

    void setVertexColor(const smColor vertColor);

    void setHighlightColor(const smColor highlightColor);

    void setNormalColor(const smColor highlightColor);

    void setShininess(const smFloat s);

    void setNormalLength(const smFloat len);

    void setDiffuseColor(const smColor diffColor);

    void setAmbientColor(const smColor ambColor);

    void setSpecularColor(const smColor specColor);

    void setShadowColor(const smColor shadColor);

    void setWireframeColor(const smColor wireColor);
    
public:
    smUInt renderType; // render type
    smColor colorDiffuse; // diffuse color
    smColor colorAmbient; // ambient color
    smColor colorSpecular; // specular color
    smColor highLightColor; // highlight color
    smColor vertexRenderColor; // vertex color
    smColor shadowColor; // shadow color
    smBool castShadow; // object can generate a shadow or not
    smBool canGetShadow; // object can get the shadow or not
    smColor normalColor; // normal color
    smColor wireFrameColor; // wire frame color
    smFloat pointSize; // point size if rendering of vertices are enabled
    smFloat lineSize; // line width size
    smFloat shininess; // specular shinness
    smBool debugDraw; // debug draw enabled or not
    smFloat normalLength; // length of rendered normals
    std::vector<std::shared_ptr<smUnifiedId>> shaders; // attached shaders
    std::vector<smBool> shaderEnable; // enable/disable any attached shader
    std::vector<std::shared_ptr<smUnifiedId>> VAOs; // stores  VAO IDs
    std::vector<smBool> VAOEnable; // enable/disable any attached VAO
};

#endif
