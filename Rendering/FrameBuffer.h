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

#ifndef SMFRAMEBUFFEROBJECT_H
#define SMFRAMEBUFFEROBJECT_H

// STL includes
#include <string>

// SimMedTK includes
#include "Core/Config.h"
#include "Core/CoreClass.h"
#include "Core/ErrorLog.h"
#include "External/FrameBufferObject.h"
#include "External/RenderBuffer.h"
#include "Rendering/TextureManager.h"

/// \brief render buffer type used for frame buffer attachment
class RenderBuffer: public CoreClass
{
public:
    /// \brief render buffer type
    enum Type
    {
        Depth = GL_DEPTH_COMPONENT,
        Stencil = GL_STENCIL_INDEX,
        ColorRGBA = GL_RGBA,
        ColorRGB = GL_RGB
    };

protected:
    /// \brief renderbuffer
    Renderbuffer _rb;
    /// \brief width of renderbuffer
    int width;
    /// \brief height of the render buffer
    int height;
    /// \brief allocation is done or not
    bool isAllocated;
    /// \brief type of renderbuffer
    Type type;
    /// \brief attachment order in the frame buffer
    int attachmentOrder;

public:
    /// \brief get height
    int getHeight();
    /// \brief get width
    int getWidth();
    /// \brief set the attachment oder
    void setAttachmentOrder(int p_attachmentOrder);
    /// \brief get the attachment order
    int getAttachmentOrder(int p_attachmentOrder);
    /// \brief get attacnment id. returns GL binding
    GLenum getGLAttachmentId();
    /// \brief returns buffer type
    Type getRenderBufType();
    /// \brief return GL buffer id
    GLuint  getRenderBufId();
    /// \brief defaul constructor.
    RenderBuffer();
    /// \brief set the type
    RenderBuffer(Type p_type, int p_width, int p_height);
    /// \brief create a depth buffer
    bool createDepthBuffer(int width, int height);
    /// \brief create a color buffer
    bool createColorBuffer();
    /// \brief create a stencil buffer
    bool createStencilBuffer();
};

/// \brief GL frame buffer class
class FrameBuffer: public CoreClass
{
public:
    /// \brief GL frame buffer
    FramebufferObject _fbo;
    /// \brief   color buffer is attached or not
    bool isColorBufAttached;
    /// \brief   depth buffer is attached or not
    bool isDepthTexAttached;
    /// \brief default color buffer attachment
    int defaultColorAttachment;
    /// \brief render buffer pointer
    RenderBuffer *renderBuffer;
    /// \brief it is enabled when the renderbuffer exists
    bool renderDepthBuff;
    /// \brief it is enabled when the color buffer exists
    bool renderColorBuff;
    /// \brief width
    int width;
    /// \brief height
    int height;

    /// \brief framebuffer default constructor
    FrameBuffer();
    virtual ~FrameBuffer();
    /// \brief set dimension of the renderbuffer
    void setDim(int p_width, int p_height);
    /// \brief get height of the framebuffer
    int getHeight();
    /// \brief get the widht of the renderbuffer
    int getWidth();
    /// \brief attach texture
    void attachTexture();
    /// \brief attach render buffer to te frame buffer
    void attachRenderBuffer(RenderBuffer *p_renderBuf);
    /// \brief attach depth texture
    void attachDepthTexture(Texture *p_texture);
    /// \brief attach a color texture
    void attachColorTexture(Texture *p_texture, int p_attachmentOrder);
    /// \brief activate color buffer in the specified order
    void activeColorBuf(int  p_order);
    /// \brief enable the framebuffer
    void enable();
    /// \brief disable the frame buffer
    void disable();
    /// \brief check status of the frame buffer. It returns ok if the frame buffer is complete
    bool checkStatus();
    /// \brief draw framebuffer. it is for debug purposes.
    void draw() const override;
};

#endif
