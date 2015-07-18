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
#include "Core/Config.h"
#include "Core/Color.h"
#include "Core/RenderDetail.h"


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
#define    SIMMEDTK_VIEWERRENDER_GLOBAL_AXIS                    (1<<13)


/// \brief viewer detail. legacy code
struct smViewerDetail
{
    smUInt environment;
    smColor backGroundColor;
};

#endif // SMCONFIGRENDERING_H
