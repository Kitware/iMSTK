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

#include "Rendering/FrameBuffer.h"

#include <string.h>
#include "Core/Config.h"
#include "Rendering/TextureManager.h"
#include "Rendering/GLRenderer.h"

bool FrameBuffer::checkStatus()
{

    GLenum ret = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    switch (ret)
    {
    case GL_FRAMEBUFFER_COMPLETE_EXT:
        std::cout << "Framebuffer complete." << std::endl;
        return true;

    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
        std::cout << "[ERROR] Framebuffer incomplete: Attachment is NOT complete." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
        std::cout << "[ERROR] Framebuffer incomplete: No image is attached to FBO." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
        std::cout << "[ERROR] Framebuffer incomplete: Attached images have different dimensions." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
        std::cout << "[ERROR] Framebuffer incomplete: Color attached images have different internal formats." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
        std::cout << "[ERROR] Framebuffer incomplete: Draw buffer." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
        std::cout << "[ERROR] Framebuffer incomplete: Read buffer." << std::endl;
        return false;

    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
        std::cout << "[ERROR] Unsupported by FBO implementation." << std::endl;
        return false;

    default:
        std::cout << "[ERROR] Unknow error." << std::endl;
        return false;
    }
}

void test()
{

    glPushAttrib(GL_TEXTURE_BIT | GL_VIEWPORT_BIT | GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, 1, 20);
    glViewport(0, 0, 1680, 1050);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor4f(1, 1, 1, 1);
    glTranslated(0, 0, -1);
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    glTexCoord2d(0, 0);
    glVertex3d(-1, -1, 0);
    glTexCoord2d(1, 0);
    glVertex3d(1, -1, 0);
    glTexCoord2d(1, 1);
    glVertex3d(1, 1.0, 0);
    glTexCoord2d(0, 1);
    glVertex3d(-1, 1.0, 0);
    glEnd();
    glPopAttrib();
}

void FrameBuffer::draw() const
{

    glPushAttrib(GL_TEXTURE_BIT | GL_VIEWPORT_BIT | GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, 1, 20);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor4f(1, 1, 1, 1);
    glTranslated(0, 0, -1);
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    glTexCoord2d(0, 0);
    glVertex3d(-1, -1, 0);
    glTexCoord2d(1, 0);
    glVertex3d(1, -1, 0);
    glTexCoord2d(1, 1);
    glVertex3d(1, 1.0, 0);
    glTexCoord2d(0, 1);
    glVertex3d(-1, 1.0, 0);
    glEnd();
    glPopAttrib();
}
GLenum RenderBuffer::getGLAttachmentId()
{
    if ( type == Type::Depth )
    {
        return GL_DEPTH_ATTACHMENT_EXT;
    }

    if ( type == Type::Stencil )
    {
        return GL_STENCIL_ATTACHMENT;
    }

    if ( type == Type::ColorRGBA || type == Type::ColorRGB )
    {
        return GL_COLOR_ATTACHMENT0_EXT + attachmentOrder;
    }
    return GLenum(0);
}
int RenderBuffer::getHeight()
{
    return height;
}
int RenderBuffer::getWidth()
{
    return width;
}
void RenderBuffer::setAttachmentOrder( int p_attachmentOrder )
{
    attachmentOrder = p_attachmentOrder;
}
int RenderBuffer::getAttachmentOrder( int /*p_attachmentOrder*/ )
{
    return attachmentOrder;
}
RenderBuffer::Type RenderBuffer::getRenderBufType()
{
    return type;
}
GLuint RenderBuffer::getRenderBufId()
{
    return _rb.GetId();
}
RenderBuffer::RenderBuffer()
{
    isAllocated = false;
}
RenderBuffer::RenderBuffer( Type p_type, int p_width, int p_height )
{
    width = p_width;
    height = p_height;
    _rb.Set( p_type, width, height );
    isAllocated = true;
    type = p_type;
}
bool RenderBuffer::createDepthBuffer( int width, int height )
{
    if ( !isAllocated )
    {
        _rb.Set( GL_DEPTH_COMPONENT, width, height );
        return true;
    }

    return false;
}
bool RenderBuffer::createColorBuffer()
{
    if ( !isAllocated )
    {
        _rb.Set( Type::ColorRGBA, width, height );
    }

    return false;
}
bool RenderBuffer::createStencilBuffer()
{
    if ( !isAllocated )
    {
        _rb.Set( Type::Stencil, width, height );
    }

    return false;
}
FrameBuffer::FrameBuffer()
{
    width = 0;
    height = 0;
    isColorBufAttached = false;
    isDepthTexAttached = false;
    renderDepthBuff = false;
    renderColorBuff = false;
    renderBuffer = nullptr;
}
void FrameBuffer::setDim( int p_width, int p_height )
{
    width = p_width;
    height = p_height;
}
int FrameBuffer::getHeight()
{
    return height;
}
int FrameBuffer::getWidth()
{
    return width;
}
void FrameBuffer::attachRenderBuffer( RenderBuffer *p_renderBuf )
{
    if ( p_renderBuf->getWidth() != width || p_renderBuf->getHeight() != height )
    {
        _fbo.AttachRenderBuffer( p_renderBuf->getRenderBufId(), p_renderBuf->getGLAttachmentId() );
    }
}
void FrameBuffer::attachDepthTexture( Texture *p_texture )
{
    if ( p_texture == nullptr )
    {
        std::cout << "Error in frambuffer depth attachment" << "\n";
    }

    _fbo.AttachTexture( p_texture->GLtype, p_texture->textureGLId, GL_DEPTH_ATTACHMENT_EXT );
    isDepthTexAttached = true;
}
void FrameBuffer::attachColorTexture( Texture *p_texture, int p_attachmentOrder )
{
    defaultColorAttachment = p_attachmentOrder;
    _fbo.AttachTexture( p_texture->GLtype, p_texture->textureGLId, GL_COLOR_ATTACHMENT0_EXT + p_attachmentOrder );
    isColorBufAttached = true;
}
void FrameBuffer::activeColorBuf( int p_order )
{
    glDrawBuffer( GL_COLOR_ATTACHMENT0_EXT + p_order );
}
void FrameBuffer::enable()
{
    _fbo.Bind();

    if ( !isColorBufAttached )
    {
        glDrawBuffer( GL_NONE );
        glReadBuffer( GL_NONE );
    }
}
void FrameBuffer::disable()
{
    _fbo.Disable();
}
FrameBuffer::~FrameBuffer() {}
