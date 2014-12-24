#include "smShader/SceneTextureShader.h"
#include "smCore/smSDK.h"


SceneTextureShader::SceneTextureShader(smChar *p_verteShaderFileName, smChar *p_fragmentFileName)
{

    this->log = smSDK::getErrorLog();
    this->log->isOutputtoConsoleEnabled = false;
    this->checkErrorEnabled = true;
    setShaderFileName(p_verteShaderFileName, NULL, p_fragmentFileName);
    createParam("depthTex");
    createParam("sceneTex");
    createParam("prevTex");
    this->checkErrorEnabled = true;
    log->isOutputtoConsoleEnabled = true;
    this->registerShader();
}


void SceneTextureShader::predraw(smMesh *p_mesh)
{

}

void SceneTextureShader::handleEvent(smEvent *p_event)
{

}

void SceneTextureShader::initDraw(smDrawParam p_param)
{

    smShader::initDraw(p_param);
    this->depthTex = getFragmentShaderParam("depthTex");
    this->sceneTex = getFragmentShaderParam("sceneTex");
    this->prevTex = getFragmentShaderParam("prevTex");
}

void SceneTextureShader::draw(smDrawParam p_param)
{

    glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);
    glDisable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, 1, 20);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor4f(1, 1, 1, 1);

    glTranslated(0, 0, -5);
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    glTexCoord2f(0, 0);
    glVertex3f(-1, -1, 0);
    glTexCoord2f(1, 0);
    glVertex3f(1, -1, 0);
    glTexCoord2f(1, 1);
    glVertex3f(1, 1.0, 0);
    glTexCoord2f(0, 1);
    glVertex3f(-1, 1.0, 0);
    glEnd();
    glPopAttrib();
}
