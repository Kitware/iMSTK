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

#ifndef CORE_CONFIGRENDERING_H
#define CORE_CONFIGRENDERING_H

/// \brief this shows the Vertex Buffer Object Size
///It should be bigger than the total data of vertices and indices of the scene objects.
///the value is in bytes
#define IMSTK_VBOBUFFER_DATASIZE 500000
#define IMSTK_VBOBUFFER_INDEXSIZE    100000
#define IMSTK_VIEWER_MAXLIGHTS   32

#define IMSTK_MAX_TEXTURENBR 36
#define IMSTK_MAX_VBOBUFFERS 10

/// \brief The configuration for VBO
enum VBOType
{
    IMSTK_VBO_STATIC,
    IMSTK_VBO_NOINDICESCHANGE,
    IMSTK_VBO_DYNAMIC,
};

/// \brief Vertex Buffer Objects Return Types
enum VBOResult
{
    IMSTK_VBO_OK,
    IMSTK_VBO_NODATAMEMORY,
    IMSTK_VBO_NOINDEXMEMORY,
    IMSTK_VBO_INVALIDOPERATION,
    IMSTK_VBO_BUFFERPOINTERERROR
};

/// \brief renderType. this shows how the render will be done
#define    IMSTK_RENDER_TEXTURE           (1<<1)
#define    IMSTK_RENDER_MATERIALCOLOR     (1<<2)
#define    IMSTK_RENDER_SHADOWS           (1<<3)
#define    IMSTK_RENDER_COLORMAP          (1<<4)
#define    IMSTK_RENDER_WIREFRAME         (1<<5)
#define    IMSTK_RENDER_VERTICES          (1<<6)
#define    IMSTK_RENDER_HIGHLIGHTVERTICES (1<<7)
#define    IMSTK_RENDER_TRANSPARENT       (1<<8)
#define    IMSTK_RENDER_LOCALAXIS         (1<<9)
#define    IMSTK_RENDER_HIGHLIGHT         (1<<10)
#define    IMSTK_RENDER_TETRAS            (1<<11)
#define    IMSTK_RENDER_SURFACE           (1<<12)
#define    IMSTK_RENDER_CUSTOMRENDERONLY  (1<<13)
#define    IMSTK_RENDER_SHADERTEXTURE     (1<<14)
#define    IMSTK_RENDER_FACES             (1<<15)
///Renders Vertex Buffer Objects
#define    IMSTK_RENDER_VBO               (1<<16)
#define    IMSTK_RENDER_NORMALS           (1<<17)
#define    IMSTK_RENDER_NONE              (1<<31)

/// \brief type definitions for variable viewerRenderDetail in Viewer
#define    IMSTK_VIEWERRENDER_GLOBALAXIS                     (1<<1)
#define    IMSTK_VIEWERRENDER_TEXT                           (1<<2)
#define    IMSTK_VIEWERRENDER_FADEBACKGROUND                 (1<<3)
#define    IMSTK_VIEWERRENDER_FADEBACKGROUNDIMAGE            (1<<4)
#define    IMSTK_VIEWERRENDER_VBO_ENABLED                    (1<<5)
#define    IMSTK_VIEWERRENDER_WIREFRAMEALL                   (1<<6)
#define    IMSTK_VIEWERRENDER_TRANSPARENCY                   (1<<7)
#define    IMSTK_VIEWERRENDER_FULLSCREEN                     (1<<8)
#define    IMSTK_VIEWERRENDER_RESTORELASTCAMSETTINGS         (1<<9)
#define    IMSTK_VIEWERRENDER_DISABLE                        (1<<11)
#define    IMSTK_VIEWERRENDER_DYNAMICREFLECTION              (1<<12)
#define    IMSTK_VIEWERRENDER_GLOBAL_AXIS                    (1<<13)
#define    IMSTK_DISABLE_MOUSE_INTERACTION                   (1<<14)

#endif // CORE_CONFIGRENDERING_H
