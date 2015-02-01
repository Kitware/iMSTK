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

#include <string.h>
#include "smRendering/smFrameBuffer.h"
#include "smCore/smConfig.h"
#include "smCore/smTextureManager.h"
#include "smRendering/smGLRenderer.h"

smBool smFrameBuffer::checkStatus()
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
    glVertex3f(-1, -1, 0);
    glTexCoord2d(1, 0);
    glVertex3f(1, -1, 0);
    glTexCoord2d(1, 1);
    glVertex3f(1, 1.0, 0);
    glTexCoord2d(0, 1);
    glVertex3f(-1, 1.0, 0);
    glEnd();
    glPopAttrib();
}

void smFrameBuffer::draw(smDrawParam p_params)
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
    glVertex3f(-1, -1, 0);
    glTexCoord2d(1, 0);
    glVertex3f(1, -1, 0);
    glTexCoord2d(1, 1);
    glVertex3f(1, 1.0, 0);
    glTexCoord2d(0, 1);
    glVertex3f(-1, 1.0, 0);
    glEnd();
    glPopAttrib();
}
