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

#ifndef SMFRAMEBUFFEROBJECT_H
#define SMFRAMEBUFFEROBJECT_H

#include <string.h>
#include <QVector>

#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smCore/smErrorLog.h"
#include "smExternal/framebufferObject.h"
#include "smExternal/renderbuffer.h"
#include "smCore/smTextureManager.h"
/// \brief frame buffer attachment type; color, depth, stencil
enum smFBOImageAttachmentType
{
    SIMMEDTK_FBOIMAGE_COLOR,
    SIMMEDTK_FBOIMAGE_DEPTH,
    SIMMEDTK_FBOIMAGE_STENCIL
};
/// \brief render buffer type
enum smRenderBufferType
{
    SIMMEDTK_RENDERBUFFER_DEPTH = GL_DEPTH_COMPONENT,
    SIMMEDTK_RENDERBUFFER_STENCIL = GL_STENCIL_INDEX,
    SIMMEDTK_RENDERBUFFER_COLOR_RGBA = GL_RGBA,
    SIMMEDTK_RENDERBUFFER_COLOR_RGB = GL_RGB
};
/// \brief render buffer type used for frame buffer attachment
class smRenderBuffer: public smCoreClass
{
protected:
    /// \brief renderbuffer
    Renderbuffer _rb;
    /// \brief width of renderbuffer
    smInt width;
    /// \brief height of the render buffer
    smInt height;
    /// \brief allocation is done or not
    smBool isAllocated;
    /// \brief type of renderbuffer
    smRenderBufferType type;
    /// \brief attachment order in the frame buffer
    smInt attachmentOrder;

public:
    /// \brief get height
    inline smInt getHeight()
    {
        return height;
    }
    /// \brief get width
    inline smInt getWidth()
    {
        return width;
    }
    /// \brief set the attachment oder
    void setAttachmentOrder(smInt p_attachmentOrder)
    {
        attachmentOrder = p_attachmentOrder;
    }
    /// \brief get the attachment order
    smInt getAttachmentOrder(smInt p_attachmentOrder)
    {
        return attachmentOrder;
    }
    /// \brief get attacnment id. returns GL binding
    GLenum getGLAttachmentId()
    {
        if (type == SIMMEDTK_RENDERBUFFER_DEPTH)
        {
            return GL_DEPTH_ATTACHMENT_EXT;
        }

        if (type == SIMMEDTK_RENDERBUFFER_STENCIL)
        {
            return GL_STENCIL_ATTACHMENT;
        }

        if (type == SIMMEDTK_RENDERBUFFER_COLOR_RGBA || type == SIMMEDTK_RENDERBUFFER_COLOR_RGB)
        {
            return GL_COLOR_ATTACHMENT0_EXT + attachmentOrder;
        }
    }
    /// \brief returns buffer type
    inline smRenderBufferType getRenderBufType()
    {
        return type;
    }
    /// \brief return GL buffer id
    inline smGLUInt  getRenderBufId()
    {
        return _rb.GetId();
    }
    /// \brief defaul constructor.
    smRenderBuffer()
    {
        isAllocated = false;
    }
    /// \brief set the type
    smRenderBuffer(smRenderBufferType p_type, smInt p_width, smInt p_height)
    {
        width = p_width;
        height = p_height;
        _rb.Set(p_type, width, height);
        isAllocated = true;
        type = p_type;
    }
    /// \brief create a depth buffer
    smBool createDepthBuffer(smInt width, smInt height)
    {
        if (!isAllocated)
        {
            _rb.Set(GL_DEPTH_COMPONENT, width, height);
            return true;
        }

        return false;
    }
    /// \brief create a color buffer
    smBool createColorBuffer()
    {
        if (!isAllocated)
        {
            _rb.Set(SIMMEDTK_RENDERBUFFER_COLOR_RGBA, width, height);
        }

        return false;
    }
    /// \brief create a stencil buffer
    smBool createStencilBuffer()
    {
        if (!isAllocated)
        {
            _rb.Set(SIMMEDTK_RENDERBUFFER_STENCIL, width, height);
        }

        return false;
    }
};

/// \brief GL frame buffer class
class smFrameBuffer: public smCoreClass
{
public:
    /// \brief GL frame buffer
    FramebufferObject _fbo;
    /// \brief   color buffer is attached or not
    smBool isColorBufAttached;
    /// \brief   depth buffer is attached or not
    smBool isDepthTexAttached;
    /// \brief default color buffer attachment
    smInt defaultColorAttachment;
    /// \brief render buffer pointer
    smRenderBuffer *renderBuffer;
    /// \brief it is enabled when the renderbuffer exists
    smBool renderDepthBuff;
    /// \brief it is enabled when the color buffer exists
    smBool renderColorBuff;
    /// \brief width
    smInt width;
    /// \brief height
    smInt height;

    /// \brief framebuffer default constructor
    smFrameBuffer()
    {
        width = 0;
        height = 0;
        isColorBufAttached = false;
        isDepthTexAttached = false;
        renderDepthBuff = false;
        renderColorBuff = false;
        renderBuffer = NULL;
    }
    /// \brief set dimension of the renderbuffer
    void setDim(smInt p_width, smInt p_height)
    {
        width = p_width;
        height = p_height;
    }
    /// \brief get height of the framebuffer
    inline smInt getHeight()
    {
        return height;
    }
    /// \brief get the widht of the renderbuffer
    inline smInt getWidth()
    {
        return width;
    }
    /// \brief attach texture
    void attachTexture();
    /// \brief attach render buffer to te frame buffer
    void attachRenderBuffer(smRenderBuffer *p_renderBuf)
    {
        if (p_renderBuf->getWidth() != width || p_renderBuf->getHeight() != height)
        {
            _fbo.AttachRenderBuffer(p_renderBuf->getRenderBufId(), p_renderBuf->getGLAttachmentId());
        }
    }
    /// \brief attach depth texture
    void attachDepthTexture(smTexture *p_texture)
    {
        if (p_texture == NULL)
        {
            cout << "Error in frambuffer depth attachment" << endl;
        }

        _fbo.AttachTexture(p_texture->GLtype, p_texture->textureGLId, GL_DEPTH_ATTACHMENT_EXT);
        isDepthTexAttached = true;
    }
    /// \brief attach a color texture
    void attachColorTexture(smTexture *p_texture, smInt p_attachmentOrder)
    {
        defaultColorAttachment = p_attachmentOrder;
        _fbo.AttachTexture(p_texture->GLtype, p_texture->textureGLId, GL_COLOR_ATTACHMENT0_EXT + p_attachmentOrder);
        isColorBufAttached = true;
    }
    /// \brief activate color buffer in the specified order
    inline void activeColorBuf(smInt  p_order)
    {
        glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT + p_order);
    }
    /// \brief enable the framebuffer
    inline void enable()
    {
        _fbo.Bind();

        if (!isColorBufAttached)
        {
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
        }
    }
    /// \brief disable the frame buffer
    inline void disable()
    {
        _fbo.Disable();
    }
    /// \brief check status of the frame buffer. It returns ok if the frame buffer is complete
    smBool checkStatus();
    /// \brief draw framebuffer. it is for debug purposes.
    void draw(smDrawParam p_params);
};

#endif
