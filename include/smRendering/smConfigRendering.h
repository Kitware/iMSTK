/*=========================================================================
 * Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
 *                        Rensselaer Polytechnic Institute
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 /=========================================================================
 
 /**
  *  \brief
  *  \details
  *  \author
  *  \author
  *  \copyright Apache License, Version 2.0.
  */

#ifndef SMCONFIGRENDERING_H
#define SMCONFIGRENDERING_H

#include "smCore/smConfig.h"

#ifdef SIMMEDTK_RENDERER_OPENGL
#include <GL/glew.h>
#include <GL/glut.h>
#endif

#include <QString>
#include <QVector>

#include "math.h"
#include "smUtilities/smVec3.h"

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
    smColor()
    {
        rgba[0] = 0.8f;
        rgba[1] = 0.8f;
        rgba[2] = 0.8f;
        rgba[3] = 1.0f;
    }
    smColor(smFloat r, smFloat g, smFloat b, smFloat a = 1.0)
    {
        rgba[0] = r;
        rgba[1] = g;
        rgba[2] = b;
        rgba[3] = a;
    }

    /// \brief Dark ratio. the valu is between 0 and 1.0
    void darken(smFloat p_darkFactor)
    {

        rgba[0] = (rgba[1] - rgba[1] * (p_darkFactor));
        rgba[1] = (rgba[2] - rgba[2] * (p_darkFactor));
        rgba[2] = (rgba[3] - rgba[3] * (p_darkFactor));
        rgba[0] = (rgba[0] < 0 ? 0 : rgba[0]);
        rgba[1] = (rgba[1] < 0 ? 0 : rgba[1]);
        rgba[2] = (rgba[2] < 0 ? 0 : rgba[2]);
    }
    /// \brief lighten the color
    void lighten(smFloat p_darkFactor)
    {

        rgba[0] = rgba[1] + rgba[1] * (p_darkFactor);
        rgba[1] = rgba[2] + rgba[2] * (p_darkFactor);
        rgba[2] = rgba[3] + rgba[3] * (p_darkFactor);

        rgba[0] = (rgba[0] > 1.0 ? 1.0 : rgba[0]);
        rgba[1] = (rgba[1] < 1.0 ? 1.0 : rgba[1]);
        rgba[2] = (rgba[2] < 1.0 ? 1.0 : rgba[2]);
    }

    /// \brief returns the color value given with the index
    smFloat operator()(smInt p_i)
    {
        if (p_i < 0 || p_i > 3)
        {
            return -1;
        }

        return rgba[p_i];
    }
    /// \brief setting
    smColor &operator=(smColor &p_color)
    {
        rgba[0] = p_color.rgba[0];
        rgba[1] = p_color.rgba[1];
        rgba[2] = p_color.rgba[2];
        rgba[3] = p_color.rgba[3];
        return *this;
    }
    /// \brief converts to gl color
    smGLFloat* toGLColor()
    {
        return (smGLFloat*)rgba;
    }
    /// \brief set RGB color
    inline void setValue(smFloat p_red, smFloat p_green, smFloat p_blue, smFloat p_alpha)
    {
        rgba[0] = p_red;
        rgba[1] = p_green;
        rgba[2] = p_blue;
        rgba[3] = p_alpha;
    }

    static smColor colorWhite;
    static smColor colorBlue;
    static smColor colorGreen;
    static smColor colorRed;
    static smColor colorGray;
    static smColor colorPink;
    static smColor colorYellow;
};


/// \brief renderType. this shows how the render will be done
#define    SIMMEDTK_RENDER_TEXTURE          (1<<1)
#define    SIMMEDTK_RENDER_MATERIALCOLOR        (1<<2)
#define    SIMMEDTK_RENDER_SHADOWS          (1<<3)
#define    SIMMEDTK_RENDER_COLORMAP         (1<<4)
#define    SIMMEDTK_RENDER_WIREFRAME            (1<<5)
#define    SIMMEDTK_RENDER_VERTICES         (1<<6)
#define    SIMMEDTK_RENDER_HIGHLIGHTVERTICES    (1<<7)
#define    SIMMEDTK_RENDER_TRANSPARENT      (1<<8)
#define    SIMMEDTK_RENDER_LOCALAXIS            (1<<9)
#define    SIMMEDTK_RENDER_HIGHLIGHT            (1<<10)
#define    SIMMEDTK_RENDER_TETRAS               (1<<11)
#define    SIMMEDTK_RENDER_SURFACE          (1<<12)
#define    SIMMEDTK_RENDER_CUSTOMRENDERONLY (1<<13)
#define    SIMMEDTK_RENDER_SHADERTEXTURE        (1<<14)
#define    SIMMEDTK_RENDER_FACES                (1<<15)
///Renders Vertex Buffer Objects
#define    SIMMEDTK_RENDER_VBO              (1<<16)
#define    SIMMEDTK_RENDER_NORMALS          (1<<17)
#define    SIMMEDTK_RENDER_VAO              (1<<18)
#define    SIMMEDTK_RENDER_NONE             (1<<31)

/// \brief type definitions for variable viewerRenderDetail in smViewer
#define    SIMMEDTK_VIEWERRENDER_GLOBALAXIS                     (1<<1)
#define    SIMMEDTK_VIEWERRENDER_TEXT                               (1<<2)
#define    SIMMEDTK_VIEWERRENDER_FADEBACKGROUND                 (1<<3)
#define    SIMMEDTK_VIEWERRENDER_FADEBACKGROUNDIMAGE                (1<<4)
#define    SIMMEDTK_VIEWERRENDER_GROUND                         (1<<5)
#define    SIMMEDTK_VIEWERRENDER_VBO_ENABLED                        (1<<6)
#define    SIMMEDTK_VIEWERRENDER_WIREFRAMEALL                       (1<<7)
#define    SIMMEDTK_VIEWERRENDER_TRANSPARENCY                       (1<<8)
#define    SIMMEDTK_VIEWERRENDER_FULLSCREEN                     (1<<9)
#define    SIMMEDTK_VIEWERRENDER_RESTORELASTCAMSETTINGS         (1<<10)
#define    SIMMEDTK_VIEWERRENDER_SOFTSHADOWS                        (1<<11)
#define    SIMMEDTK_VIEWERRENDER_DISABLE                            (1<<12)
#define    SIMMEDTK_VIEWERRENDER_DYNAMICREFLECTION              (1<<13)




/// \brief viewer detail. legacy code
struct smViewerDetail
{
    smUInt environment;
    smColor backGroundColor;
};

struct smUnifiedID;


/// \brief smRenderDetail has rendering options and features.
///It shows how the mesh should be rendered
struct smRenderDetail
{
public:
    smRenderDetail() /*:shaders(10)*/
    {
        renderType = SIMMEDTK_RENDER_MATERIALCOLOR | SIMMEDTK_RENDER_FACES;
        highLightColor.rgba[0] = 1.0f;
        highLightColor.rgba[1] = 0.0f;
        highLightColor.rgba[2] = 0.0f;
        pointSize = 1;
        lineSize = 1;
        shadowColor.rgba[0] = 0.0f;
        shadowColor.rgba[1] = 0.0f;
        shadowColor.rgba[2] = 0.0f;
        shadowColor.rgba[3] = 0.5f;
        colorDiffuse = smColor::colorWhite;
        colorAmbient = smColor::colorWhite;
        colorSpecular = smColor::colorWhite;
        normalColor = smColor::colorGreen;
        wireFrameColor = smColor::colorBlue;
        shininess = 50.0;
        debugDraw = false;
        castShadow = true;
        canGetShadow = true;
    }
    /// \brief render type
    smUInt renderType;
    /// \brief diffuse color
    smColor colorDiffuse;
    /// \brief ambient color
    smColor colorAmbient;
    /// \brief specular color
    smColor colorSpecular;
    /// \brief highlight color
    smColor highLightColor;
    /// \brief shadow color
    smColor shadowColor;
    /// \brief object can generate a shadow or not
    smBool castShadow;
    /// \brief object can get the shadow or not
    smBool canGetShadow;
    /// \brief normal color
    smColor normalColor;
    /// \brief wire frame color
    smColor wireFrameColor;
    /// \brief point size if rendering of vertices are enabled
    smFloat pointSize;
    /// \brief line width size
    smFloat lineSize;
    /// \brief specular shinness
    smFloat shininess;
    /// \brief debug draw enabled or not
    smBool debugDraw;
    /// \brief attachment of shader
    void addShader(smUnifiedID p_shaderID);
    /// \brief attachment of VAO
    void addVAO(smUnifiedID p_shaderID);
    /// \brief attached shaders
    vector <smUnifiedID >shaders;
    /// \brief enable/disable any attached shader
    vector<smChar>shaderEnable;
    /// \brief stores  VAO IDs
    vector <smUnifiedID>VAOs;
    /// \brief enable/disable any attached VAO
    vector<smChar>VAOEnable;
};

#endif
