#include "smUtilities/smGLUtils.h"
#include "smShader/smShader.h"

struct smGroundRenderInfo
{
    smChar groundImage[255];
    smChar groundBumpImage[255];
    smChar wallImage[255];
    smChar wallBumpImage[255];
    smGLInt attributeImage;
    smGLInt attributeBumpImage;
    smGLInt tangent;
    smShader *bumpMappingShader;
    smFloat scale;
};

smGroundRenderInfo smGLUtils::groundRenderInfo;

void smGLUtils::init()
{
    groundRenderInfo.bumpMappingShader = new smShader(NULL);
    groundRenderInfo.bumpMappingShader->initShaders("../../resources/shaders/VertexBumpMap.cg", "../../resources/shaders/FragmentBumpMap.cg", NULL);
    groundRenderInfo.tangent = groundRenderInfo.bumpMappingShader->addShaderParamAttrib("tangent");
    groundRenderInfo.attributeImage = groundRenderInfo.bumpMappingShader->addShaderParamForAll("DecalTex");
    groundRenderInfo.attributeBumpImage = groundRenderInfo.bumpMappingShader->addShaderParamForAll("BumpTex");
    smTextureManager::loadTexture(groundRenderInfo.groundImage, "groundImage");
    smTextureManager::loadTexture(groundRenderInfo.groundBumpImage, "groundBumpImage");
    smTextureManager::loadTexture(groundRenderInfo.wallImage, "wallImage");
    smTextureManager::loadTexture(groundRenderInfo.wallBumpImage, "wallBumpImage");
    groundRenderInfo.scale = 10;
}

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

void smGLUtils::drawTexturedPolygon()
{
    static smFloat yOffset = -0.001;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, (smGLFloat*)&smColor::colorWhite);
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glVertexAttrib3f(groundRenderInfo.tangent, 1, 0, 0);
    glTexCoord2f(0, 1);
    glVertex3f(-1 * groundRenderInfo.scale, yOffset, -1 * groundRenderInfo.scale);

    glNormal3f(0, 1, 0);
    glVertexAttrib3f(groundRenderInfo.tangent, 1, 0, 0);
    glTexCoord2f(0, 0);
    glVertex3f(-1 * groundRenderInfo.scale, yOffset, 1 * groundRenderInfo.scale);

    glNormal3f(0, 1, 0);
    glVertexAttrib3f(groundRenderInfo.tangent, 1, 0, 0);
    glTexCoord2f(1, 0);
    glVertex3f(1 * groundRenderInfo.scale, yOffset, 1 * groundRenderInfo.scale);

    glNormal3f(0, 1, 0);
    glVertexAttrib3f(groundRenderInfo.tangent, 1, 0, 0);
    glTexCoord2f(1, 1);
    glVertex3f(1 * groundRenderInfo.scale, yOffset, -1 * groundRenderInfo.scale);
    glEnd();
}

void smGLUtils::drawGround()
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    groundRenderInfo.bumpMappingShader->enableShader();

    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    smTextureManager::activateTexture("groundImage");
    glUniform1iARB(groundRenderInfo.attributeImage, 0);

    glActiveTexture(GL_TEXTURE1);
    smTextureManager::activateTexture("groundBumpImage");
    glUniform1iARB(groundRenderInfo.attributeBumpImage, 1);
    glMatrixMode(GL_MODELVIEW);

    glPushMatrix();

    for (smInt i = -4; i < 5; i++)
    {
        glPushMatrix();
        glTranslatef(2 * groundRenderInfo.scale * i, 0, 0);
        drawTexturedPolygon();
        glTranslatef(0, 0, groundRenderInfo.scale * 2);
        drawTexturedPolygon();
        glTranslatef(0, 0, groundRenderInfo.scale * 2);
        drawTexturedPolygon();
        glPopMatrix();
    }

    glTranslatef(0, groundRenderInfo.scale, -groundRenderInfo.scale);
    glRotatef(90, 1, 0, 0);
    glEnable(GL_TEXTURE_2D);

    glActiveTexture(GL_TEXTURE0);
    smTextureManager::activateTexture("wallImage");
    glUniform1iARB(groundRenderInfo.attributeImage, 0);

    glActiveTexture(GL_TEXTURE1);
    smTextureManager::activateTexture("wallBumpImage");
    glUniform1iARB(groundRenderInfo.attributeBumpImage, 1);

    for (smInt i = -4; i < 5; i++)
    {
        glPushMatrix();
        glTranslatef(2 * groundRenderInfo.scale * i, 0, 0);
        drawTexturedPolygon();
        glTranslatef(0, 0, -2 * groundRenderInfo.scale);
        drawTexturedPolygon();
        glPopMatrix();
    }

    glPopMatrix();

    glActiveTexture(GL_TEXTURE0);
    smTextureManager::disableTexture("groundImage");

    glActiveTexture(GL_TEXTURE1);
    smTextureManager::disableTexture("groundBumpImage");
    groundRenderInfo.bumpMappingShader->disableShader();
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











