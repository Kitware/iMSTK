/*
****************************************************
SIMMEDTK LICENSE

****************************************************
*/

#ifndef SMGLUTILS_H
#define SMGLUTILS_H
#include "smCore/smConfig.h"
#include "smCore/smTextureManager.h"
#include "smUtilities/smVec3.h"
#include "smUtilities/smMatrix33.h"
#include "smUtilities/smMatrix44.h"
#include <GL/glut.h>
#include <GL/glew.h>

/// \brief stores the information about the ground
struct smGroundRenderInfo;

/// \brief opengl rendering utilities are here.
class smGLUtils : public smCoreClass
{
public:
    static smGroundRenderInfo groundRenderInfo;

    static void init();

    /// \brief checks the openGL error. if there is an error then it returns
    /// \brief the error text otherwise it returns NULL
    static bool queryGLError(smChar*err);

    /// \brief  taken from glProgramming.com.  Checks the extension.
    static smBool QueryExtension(char *extName);

    /// \brief  fade background draw with a size groundRenderInfo.scale
    static void fadeBackgroundDraw();
    /// \brief draw a textured quad with
    static void drawTexturedPolygon();
    /// \brief draw gorund
    static void  drawGround();

    /// \brief draw quad on screen with color and left, botton, right, top positions
    static void drawQuadOnScreen(smColor p_color,
                                 smFloat p_left,
                                 smFloat p_bottom,
                                 smFloat p_right,
                                 smFloat p_top);

    /// \brief draw unit quad
    static inline void drawUnitQuadOnScreen()
    {
        glBegin(GL_QUADS);
        glNormal3f(0, 0, 1);
        glTexCoord2f(0, 0);
        glVertex3f(-1, -1, -1);
        glTexCoord2f(1, 0);
        glVertex3f(1, -1, -1);
        glTexCoord2f(1, 1);
        glVertex3f(1, 1.0, -1);
        glTexCoord2f(0, 1);
        glVertex3f(-1, 1.0, -1);
        glEnd();
    }

    template <typename T>
    static inline void queryProjectionMatrix(smMatrix44<T> &p_matrix)
    {
        T m[16];
        glGetFloatv(GL_PROJECTION_MATRIX, m);
        p_matrix.setMatrixFromOpenGL(m);
    }
    /// \brief to query current model view matrix. p_matrix will have the final values.
    template <typename T>
    static inline void queryModelViewMatrix(smMatrix44<T> &p_matrix)
    {
        T m[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, m);
        p_matrix.setMatrixFromOpenGL(m);
    }
};

#endif
