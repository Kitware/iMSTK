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

#include "smUtilities/smGLUtils.h"
#include "smCore/smTextureManager.h"

///checks the openGL error. if there is an error then it returns
///the error text otherwise it returns NULL
bool smGLUtils::queryGLError(smChar*err)
{
    GLenum errCode;
    const GLubyte *errString;

    if ((errCode = glGetError()) != GL_NO_ERROR)
    {
        errString = gluErrorString(errCode);

        if (err != NULL)
        {
            sprintf(err, "OPENGL Error= %s\n", errString);
        }
        else
        {
            printf("OPENGL Error= %s\n", errString);
        }

        return err;
    }
    else
    {
        return NULL;
    }
}

///taken from glProgramming.com.  Checks the extension.
smBool smGLUtils::QueryExtension(char *extName)
{
    char *p = (char *) glGetString(GL_EXTENSIONS);
    char *end = p + strlen(p);

    while (p < end)
    {
        int n = strcspn(p, " ");

        if ((strlen(extName) == n) && (strncmp(extName, p, n) == 0))
        {
            return true;
        }

        p += (n + 1);
    }

    return false;
}

///fade background draw
void smGLUtils::fadeBackgroundDraw()
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_TEXTURE_2D);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, 0.1, 1.1);
    glBegin(GL_POLYGON);
    glColor3f(0.219, 0.219, 0.435);
    glVertex3f(-1, -1, -1);
    glColor3f(0.219, 0.219, 0.435);
    glVertex3f(1, -1, -1);
    glColor3f(0.0, 0.0, 0.0);
    glVertex3f(1, 1, -1);
    glColor3f(0.0, 0.0, 0.0);
    glVertex3f(-1, 1, -1);
    glEnd();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopAttrib();
}

void smGLUtils::drawQuadOnScreen(smColor p_color, smFloat p_left,
                                 smFloat p_bottom, smFloat p_right,
                                 smFloat p_top)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_TEXTURE_2D);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1.0, 0.0, 1, 0.1, 1.1);
    glBegin(GL_QUADS);
    glColor4fv(p_color.toGLColor());
    glVertex3f(p_left, p_bottom, -1.0);
    glVertex3f(p_right, p_bottom, -1.0);
    glVertex3f(p_right, p_top , -1.0);
    glVertex3f(p_left, p_top , -1.0);
    glEnd();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopAttrib();
}
