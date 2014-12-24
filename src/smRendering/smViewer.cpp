#include "smCore/smConfig.h"
#include "smCore/smSDK.h"
#include "smUtilities/smIOStream.h"
#include "smRendering/smGLRenderer.h"
#include "smRendering/smViewer.h"
#include "smShader/smShader.h"
#include "smUtilities/smQuat.h"
#include "smShader/metalShader.h"
#include "smShader/SceneTextureShader.h"
#include "smUtilities/smDataStructs.h"
#include "smRendering/smVBO.h"
#include "smRendering/smVAO.h"
#include "smExternal/tree.hh"

#include <QGLViewer/vec.h>
#include <QGLViewer/qglviewer.h>
#include <QKeyEvent>
#include <QDesktopWidget>

#ifdef SIMMEDTK_OPERATINGSYSTEM_LINUX
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glxew.h>
#include <GL/glxext.h>
#include <GL/glx.h>
#endif

#ifdef SIMMEDTK_OPERATINGSYSTEM_WINDOWS
typedef bool (APIENTRY *PFNWGLSWAPINTERVALFARPROC)(int);
#endif

void SetVSync(bool sync)
{

#ifdef SIMMEDTK_OPERATINGSYSTEM_WINDOWS
    PFNWGLSWAPINTERVALFARPROC wglSwapIntervalEXT = 0;
    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALFARPROC)wglGetProcAddress("wglSwapIntervalEXT");

    if (wglSwapIntervalEXT)
    {
        wglSwapIntervalEXT(sync);
    }

#elif defined(SIMMEDTK_OPERATINGSYSTEM_LINUX)
    Display *dpy = glXGetCurrentDisplay();
    GLXDrawable drawable = glXGetCurrentDrawable();
    unsigned int swap, maxSwap;

    if (drawable && dpy)
    {
        glXSwapIntervalEXT(dpy, drawable, sync);
    }

#endif
}

smViewer::smViewer(smErrorLog *log)
{

    type = SIMMEDTK_SMVIEWER;
    ::QGLViewer();
    viewerRenderDetail = SIMMEDTK_VIEWERRENDER_FADEBACKGROUND;
    shadowMatrix.setIdentity();
    shadowMatrix(0, 1) = 0;
    shadowMatrix(1, 1) = 0.0;
    shadowMatrix(2, 1) = 0;
    shadowMatrix(3, 1) = 0;

    defaultAmbientColor.setValue(0.1, 0.1, 0.1, 1.0);
    defaultDiffuseColor.setValue(0.8, 0.8, 0.8, 1.0);
    defaultSpecularColor.setValue(0.9, 0.9, 0.9, 1.0);

    this->log = log;
    consoleDisplay = false;
    lights = new smIndiceArray<smLight*>(SIMMEDTK_VIEWER_MAXLIGHTS);
    windowOutput = new smOpenGLWindowStream();
    lightDrawScale = 50;
    enableCameraMotion = false;

    boostViewer = false;

    unlimitedFPSEnabled = false;
    unlimitedFPSVariableChanged = 1;
    screenResolutionWidth = 1680;
    screenResolutionHeight = 1050;

    offsetAngle_Direction = 0;
    offsetAngle_UpDirection = 0;
    offsetAngle_rightDirection = 0;

    cameraRadius = 1.0;
    prevState_collided = false;
    checkCameraCollision = false;
}

///affects the framebuffer size and depth buffer size
void smViewer::setScreenResolution(smInt p_width, smInt p_height)
{

    this->screenResolutionHeight = p_height;
    this->screenResolutionWidth = p_width;
}

smInt smViewer::addLight(smLight *p_light)
{

    smInt index = lights->add(p_light);
    lights->getByRef(index)->renderUsage = GL_LIGHT0 + index;
    lights->getByRef(index)->activate(true);
    return index;
}

smBool smViewer::setLight(smInt p_lightId, smLight *p_light)
{

    smInt index = lights->replace(p_lightId, p_light);

    if (index > 0)
    {
        lights->getByRef(p_lightId)->renderUsage = GL_LIGHT0 + p_lightId;
        return  index;
    }
}

void smViewer::refreshLights()
{

    smIndiceArrayIter<smLight*> iter(lights);

    for (smInt i = iter.begin(); i < iter.end(); iter++)
    {
        glEnable(iter[i]->renderUsage);
        glLightfv(iter[i]->renderUsage, GL_AMBIENT, iter[i]->lightColorAmbient.toGLColor());
        glLightfv(iter[i]->renderUsage, GL_DIFFUSE, iter[i]->lightColorDiffuse.toGLColor());
        glLightfv(iter[i]->renderUsage, GL_SPECULAR, iter[i]->lightColorSpecular.toGLColor());
        glLightf(iter[i]->renderUsage, GL_SPOT_EXPONENT, iter[i]->spotExp * SMLIGHT_SPOTMAX);
        glLightf(iter[i]->renderUsage, GL_SPOT_CUTOFF, iter[i]->spotCutOffAngle);
        glLightfv(iter[i]->renderUsage, GL_POSITION, (smGLFloat*)&iter[i]->lightPos);
        glLightfv(iter[i]->renderUsage, GL_SPOT_DIRECTION, (smGLFloat*)&iter[i]->direction);
    }
}

smBool smViewer::updateLight(smInt p_lightId, smLight *p_light)
{

    p_light->updateDirection();
    return lights->replace(p_lightId, p_light);
}

void smViewer::setLightPos(smInt p_lightId, smLightPos p_pos)
{

    smLight *temp;
    temp = lights->getByRef(p_lightId);
    temp->lightPos = p_pos;
    temp->updateDirection();
}

void smViewer::setLightPos(smInt p_lightId,
                           smLightPos p_pos,
                           smVec3<smFloat> p_direction)
{

    smLight *temp;
    temp = lights->getByRef(p_lightId);
    temp->lightPos = p_pos;
    temp->direction = p_direction;
    temp->updateDirection();
}

void smViewer::setUnlimitedFPS(smBool p_enableFPS)
{

    unlimitedFPSEnabled = p_enableFPS;
    unlimitedFPSVariableChanged++;
}

///initialization of the viewer module
void smViewer::init()
{

    smSceneObject *sceneObject;
    smScene             *scene;
    smStaticSceneObject *staticSceneObject;
    smStylusRigidSceneObject *stylusObject;
    smChar *errorText = NULL;
    smClassType objectType;
    static smDrawParam param;
    smIndiceArrayIter<smLight*> iter(lights);

    param.rendererObject = this;
    param.caller = this;
    param.data = NULL;
    QGLViewer::init();

    if (isInitialized)
    {
        return;
    }

    GLenum err = glewInit();

    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong.
         * Most likely an OpenGL context is not created yet */
        cout << "Error:" << glewGetErrorString(err) << endl;
    }

    smGLUtils::queryGLError(errorText);
    setAutoBufferSwap(false);
    smBool test = doubleBuffer();

    setManipulatedFrame((qglviewer::ManipulatedFrame*)camera()->frame());
    setAnimationPeriod(0); // as fast as possible
    startAnimation();

    if (viewerRenderDetail & SIMMEDTK_VIEWERRENDER_RESTORELASTCAMSETTINGS)
    {
        restoreStateFromFile();
    }

    if (viewerRenderDetail & SIMMEDTK_VIEWERRENDER_FULLSCREEN)
    {
        setFullScreen(true);
    }

    glEnable(GL_MULTISAMPLE);
    glDisable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Create light components
    for (smInt i = iter.begin(); i < iter.end(); i++)
    {
        glEnable(iter[i]->renderUsage);
        glLightfv(iter[i]->renderUsage, GL_AMBIENT, iter[i]->lightColorAmbient.toGLColor());
        glLightfv(iter[i]->renderUsage, GL_DIFFUSE, iter[i]->lightColorDiffuse.toGLColor());
        glLightfv(iter[i]->renderUsage, GL_SPECULAR, iter[i]->lightColorSpecular.toGLColor());
        glLightf(iter[i]->renderUsage, GL_SPOT_EXPONENT, (smGLFloat)iter[i]->spotExp * SMLIGHT_SPOTMAX);
        glLightf(iter[i]->renderUsage, GL_SPOT_CUTOFF, (smGLFloat)iter[i]->spotCutOffAngle);
        glLightfv(iter[i]->renderUsage, GL_POSITION, (smGLFloat*)&iter[i]->lightPos);
        glLightfv(iter[i]->renderUsage, GL_SPOT_DIRECTION, (smGLFloat*)&iter[i]->direction);
    }

    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_MULTISAMPLE_ARB);
    glShadeModel(GL_SMOOTH);
    glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);
    glFrontFace(GL_CCW);
    glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 50);

    setAcceptDrops(true);

    for (smInt i = 0; i < objectList.size(); i++)
    {
        if (objectList[i]->getType() != SIMMEDTK_SMSHADER)
        {
            objectList[i]->initDraw(param);
        }
        else
        {
            continue;
        }
    }

    smGLUtils::init();
    smInt width = screenResolutionWidth;
    smInt height = screenResolutionHeight;

    if (viewerRenderDetail & SIMMEDTK_VIEWERRENDER_SOFTSHADOWS)
    {
        fbo = new smFrameBuffer();
        fbo->setDim(2048, 2048);
        smTextureManager::createDepthTexture("depth", 2048, 2048);
        backfbo = new smFrameBuffer();
        backfbo->setDim(1024, 1024);
        smTextureManager::createColorTexture("backmap", 1024, 1024);
        smTextureManager::createDepthTexture("backmapdepth", 1024, 1024);
        backfbo->renderDepthBuff = true;
        backfbo->renderColorBuff = true;
    }

    smTextureManager::initGLTextures();
    smShader::initGLShaders(param);
    smVAO::initVAOs(param);

    if (viewerRenderDetail & SIMMEDTK_VIEWERRENDER_SOFTSHADOWS)
    {
        fbo->setDim(2048, 2048);
        fbo->attachDepthTexture(smTextureManager::getTexture("depth"));

        cout << "Checking the status of framebuffer for shadow" << endl;
        fbo->checkStatus();

        backfbo->setDim(1024, 1024);
        backfbo->attachColorTexture(smTextureManager::getTexture("backmap"), 0);
        backfbo->attachDepthTexture(smTextureManager::getTexture("backmapdepth"));
        cout << "Checking the status of framebuffer for dualparaboloid backmap" << endl;
        backfbo->checkStatus();

        smTextureManager::disableTexture("depth");
        smTextureManager::disableTexture("backmap");
    }

    smScene::smSceneIterator sceneIter;

    //traverse all the scene and the objects in the scene
    for (smInt i = 0; i < sceneList.size(); i++)
    {
        scene = sceneList[i];
        scene->registerForScene(this);
        sceneIter.setScene(scene, this);

        for (smInt j = sceneIter.start(); j < sceneIter.end(); j++)
        {
            //sceneObject=scene->sceneObjects[j];
            sceneObject = sceneIter[j];

            //initialize the custom Render if there is any
            if (sceneObject->customRender != NULL && sceneObject->getType() != SIMMEDTK_SMSHADER)
            {
                sceneObject->customRender->initDraw(param);
            }

            sceneObject->initDraw(param);

            if (sceneObject->renderDetail.renderType & SIMMEDTK_RENDER_VBO && viewerRenderDetail & SIMMEDTK_VIEWERRENDER_VBO_ENABLED)
            {
                objectType = sceneObject->getType();

                switch (objectType)
                {
                case SIMMEDTK_SMSTATICSCENEOBJECT:
                    staticSceneObject = (smStaticSceneObject*)sceneObject;
                    break;
                }
            }//scene object is added in the vbo object.
        }//object traverse
    }//scene traverse

    cout << "Screen Width:" << camera()->screenWidth() << endl;
    cout << "Screen Height:" << camera()->screenHeight() << endl;
    cout << "Screen Res:" << QApplication::desktop()->screen(1)->width() << endl;

    isInitialized = true;

    windowOutput->addText("VFPS", QString(""));
    this->setFixedWidth(this->screenResolutionWidth);
    this->setFixedHeight(this->screenResolutionHeight);
}


///draw the surface mesh triangles based on the rendering type
///problem is here
//void smViewer::drawSurfaceMeshTriangles(smSurfaceMesh *p_surfaceMesh,smRenderDetail *renderDetail)
void smViewer::drawSurfaceMeshTriangles(smMesh *p_surfaceMesh, smRenderDetail *renderDetail)
{

    static smFloat shadowMatrixGL[16];
    static smVec3<smFloat> origin(0, 0, 0);
    static smVec3<smFloat> xAxis(1, 0, 0);
    static smVec3<smFloat> yAxis(0, 1, 0);
    static smVec3<smFloat> zAxis(0, 0, 1);

    if (renderDetail->renderType & SIMMEDTK_RENDER_NONE)
    {
        return;
    }

    glDisable(GL_TEXTURE_2D);
    glPointSize(renderDetail->pointSize);
    glLineWidth(renderDetail->lineSize);

    if (renderDetail->renderType & SIMMEDTK_RENDER_TRANSPARENT)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_MATERIALCOLOR)
    {
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, (smGLReal*)renderDetail->colorDiffuse.toGLColor());
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, (smGLReal*)renderDetail->colorSpecular.toGLColor());
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, (smGLReal*)renderDetail->colorAmbient.toGLColor());
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    if (renderDetail->renderType & SIMMEDTK_RENDER_TEXTURE)
    {
        if (p_surfaceMesh->isMeshTextured())
        {
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);

            for (smInt t = 0; t < p_surfaceMesh->textureIds.size(); t++)
            {
                glActiveTexture(GL_TEXTURE0 + t);
                smTextureManager::activateTexture(p_surfaceMesh->textureIds[t].textureId);
            }
        }
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_COLORMAP)
    {
        glEnableClientState(GL_COLOR_ARRAY);
    }

    glVertexPointer(3, smGLRealType, 0, p_surfaceMesh->vertices);

    if (renderDetail->renderType & SIMMEDTK_RENDER_TEXTURE)
    {
        if (p_surfaceMesh->isMeshTextured())
        {
            glTexCoordPointer(2, smGLRealType, 0, p_surfaceMesh->texCoord);
        }
    }

    glNormalPointer(smGLRealType, 0, p_surfaceMesh->vertNormals);

    if (renderDetail->renderType & SIMMEDTK_RENDER_FACES)
    {
        glDrawElements(GL_TRIANGLES, p_surfaceMesh->nbrTriangles * 3, smGLUIntType, p_surfaceMesh->triangles);
    }

    if ((renderDetail->renderType & (SIMMEDTK_RENDER_VERTICES)))
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        glDisable(GL_LIGHTING);
        glDrawElements(GL_TRIANGLES, p_surfaceMesh->nbrTriangles * 3, smGLUIntType, p_surfaceMesh->triangles);
        glEnable(GL_LIGHTING);
        //default rendering
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_WIREFRAME || this->viewerRenderDetail & SIMMEDTK_VIEWERRENDER_WIREFRAMEALL)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glPolygonOffset(3.0, 2.0);
        glDisable(GL_LIGHTING);
        glDrawElements(GL_TRIANGLES, p_surfaceMesh->nbrTriangles * 3, smGLUIntType, p_surfaceMesh->triangles);
        glEnable(GL_LIGHTING);
        //default rendering
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_LOCALAXIS)
    {
        glEnable(GL_LIGHTING);
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_SHADOWS)
    {
        glMatrixMode(GL_MATRIX_MODE);
        glPushMatrix();
        glDisable(GL_LIGHTING);
        shadowMatrix.getMatrixForOpenGL(shadowMatrixGL);
        glMultMatrixf((smGLReal*)shadowMatrixGL);
        glColor4fv((smGLFloat*)&renderDetail->shadowColor);
        glDrawElements(GL_TRIANGLES, p_surfaceMesh->nbrTriangles * 3, smGLUIntType, p_surfaceMesh->triangles);
        glEnable(GL_LIGHTING);
        glPopMatrix();
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_HIGHLIGHTVERTICES)
    {
        glDisable(GL_LIGHTING);
        glColor3fv((smGLReal*)&renderDetail->highLightColor);
        glDrawArrays(GL_POINTS, 0, p_surfaceMesh->nbrVertices);
        glEnable(GL_LIGHTING);
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_TRANSPARENT)
    {
        glDisable(GL_BLEND);
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);

    if (renderDetail->renderType & SIMMEDTK_RENDER_TEXTURE)
    {
        if (p_surfaceMesh->isMeshTextured())
        {
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);

            for (smInt t = 0; t < p_surfaceMesh->textureIds.size(); t++)
            {
                glActiveTexture(GL_TEXTURE0 + t);
                smTextureManager::disableTexture(p_surfaceMesh->textureIds[t].textureId);
            }
        }
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_COLORMAP)
    {
        glDisableClientState(GL_COLOR_ARRAY);
    }

    glEnable(GL_LIGHTING);
    glPointSize(1.0);
    glLineWidth(1.0);
}

///vertex buffer implementation. It is not implemented yet. It will improve the performance drastically
void smViewer::drawSurfaceMeshTrianglesVBO(smSurfaceMesh *p_surfaceMesh, smRenderDetail *renderDetail, smInt p_objectId, smVBOType p_VBOType)
{

    static smFloat shadowMatrixGL[16];
    static smVec3<smFloat> origin(0, 0, 0);
    static smVec3<smFloat> xAxis(1, 0, 0);
    static smVec3<smFloat> yAxis(0, 1, 0);
    static smVec3<smFloat> zAxis(0, 0, 1);

    if (renderDetail->renderType & SIMMEDTK_RENDER_NONE)
    {
        return;
    }

    glPointSize(renderDetail->pointSize);
    glLineWidth(renderDetail->lineSize);

    if (renderDetail->renderType & SIMMEDTK_RENDER_TRANSPARENT)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_MATERIALCOLOR)
    {
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, (smGLReal*)&renderDetail->colorDiffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, (smGLReal*)&renderDetail->colorSpecular);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, (smGLReal*)&renderDetail->colorAmbient);
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_FACES)
    {
        //placeholder
    }

    glEnable(GL_LIGHTING);
    glPointSize(1.0);
    glLineWidth(1.0);
}

void smViewer::drawSMDeformableObject(smPBDSurfaceSceneObject *p_smPhsyObject)
{

    if (p_smPhsyObject->renderDetail.renderType & SIMMEDTK_RENDER_VBO)
    {
        //placeholder
    }
    else
    {
        drawSurfaceMeshTriangles(p_smPhsyObject->mesh, &p_smPhsyObject->renderDetail);
    }

    drawNormals(p_smPhsyObject->mesh);
}

void smViewer::drawNormals(smMesh *p_mesh)
{

    glDisable(GL_LIGHTING);
    glColor3fv((smGLFloat*)&smColor::colorBlue);
    smVec3<smFloat>baryCenter;
    smGLFloat* tmp = NULL;
    glBegin(GL_LINES);

    for (smInt i = 0; i < p_mesh->nbrVertices; i++)
    {
        glVertex3fv((smGLFloat*) & (p_mesh->vertices[i]));
        smVec3<float> vector = p_mesh->vertices[i] + p_mesh->vertNormals[i] * 5;
        glVertex3fv((smGLFloat*)&vector);
    }

    for (smInt i = 0; i < p_mesh->nbrTriangles; i++)
    {
        baryCenter = p_mesh->vertices[p_mesh->triangles[i].vert[0]] + p_mesh->vertices[p_mesh->triangles[i].vert[1]] + p_mesh->vertices[p_mesh->triangles[i].vert[2]] ;
        baryCenter = baryCenter / 3.0;
        glVertex3fv((smGLFloat*) & (baryCenter));
        smVec3<float> vector = baryCenter + p_mesh->triNormals[i] * 5;
        glVertex3fv((smGLFloat*)&vector);
    }

    glEnd();

    glEnable(GL_LIGHTING);
}

///draw the static objects
void smViewer::drawSMStaticObject(smStaticSceneObject *p_smPhsyObject)
{

    if (p_smPhsyObject->renderDetail.renderType & SIMMEDTK_RENDER_VBO)
    {
        //
    }
    else
    {
        drawSurfaceMeshTriangles(p_smPhsyObject->mesh, &p_smPhsyObject->renderDetail);
    }
}

void smViewer::drawSmLight(smLight *light)
{

    glEnable(GL_LIGHTING);
    qglviewer::Vec origin(0, 0, 0);
    light->updateDirection();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    if (light->lightLocationType == SIMMEDTK_LIGHTPOS_EYE)
    {
        glLoadIdentity();
    }

    glTranslatef(light->lightPos.pos.x,
                 light->lightPos.pos.y,
                 light->lightPos.pos.z);
    glutSolidSphere(1.0, 15, 15);

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, smColor::colorBlue.toGLColor());
    drawArrow(origin, origin + qglviewer::Vec(light->direction.x, light->direction.y, light->direction.z) * 10);

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, smColor::colorRed.toGLColor());
    drawArrow(origin, origin + qglviewer::Vec(light->upVector.x, light->upVector.y, light->upVector.z) * 10);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, smColor::colorGreen.toGLColor());
    drawArrow(origin, origin + qglviewer::Vec(light->transverseDir.x, light->transverseDir.y, light->transverseDir.z) * 10);
    glPopMatrix();
}

void smViewer::enableLights()
{

    static smIndiceArrayIter<smLight*> iter(lights);
    smFloat dir[4];
    static smLightPos defaultPos(0, 0, 0);

    glEnable(GL_LIGHTING);

    for (smInt i = iter.begin(); i < iter.end(); i++)
    {
        if (iter[i]->isEnabled())
        {
            glEnable(iter[i]->renderUsage);
        }
        else
        {
            glDisable(iter[i]->renderUsage);
        }

        glLightf(iter[i]->renderUsage, GL_CONSTANT_ATTENUATION, iter[i]->attn_constant);
        glLightf(iter[i]->renderUsage, GL_LINEAR_ATTENUATION, iter[i]->attn_linear);
        glLightf(iter[i]->renderUsage, GL_QUADRATIC_ATTENUATION, iter[i]->attn_quadratic);

        if (iter[i]->lightLocationType == SIMMEDTK_LIGHTPOS_EYE)
        {
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            glLightfv(iter[i]->renderUsage, GL_POSITION, (smGLFloat*)&iter[i]->lightPos);
            glPopMatrix();
        }
        else
        {
            glLightfv(iter[i]->renderUsage, GL_POSITION, (smGLFloat*)&iter[i]->lightPos);
        }

        if (iter[i]->lightType == SIMMEDTK_LIGHT_SPOTLIGHT)
        {
            glLightfv(iter[i]->renderUsage, GL_SPOT_DIRECTION, (smGLFloat*)&iter[i]->direction);
        }

        glGetLightfv(iter[i]->renderUsage, GL_SPOT_DIRECTION, dir);

        if (iter[i]->drawEnabled)
        {
            drawSmLight(iter[i]);
        }
    }
}

void smViewer::renderScene(smDrawParam p_param)
{

    smSceneObject *sceneObject;

    if (this->renderStage == SMRENDERSTAGE_FINALPASS)
    {
        glDisable(GL_CULL_FACE);
    }

    if (viewerRenderDetail & SIMMEDTK_VIEWERRENDER_GLOBALAXIS)
    {
        glPushAttrib(GL_COLOR_BUFFER_BIT);
        drawAxis(30);
        glPopAttrib();
    }

    if (viewerRenderDetail & SIMMEDTK_VIEWERRENDER_TRANSPARENCY)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_POLYGON_OFFSET_FILL);
    }
    else
    {
        glDisable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    if (viewerRenderDetail & SIMMEDTK_VIEWERRENDER_GROUND)
    {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glColor3f(1, 1, 1);
        smGLUtils::drawGround();
        glEnable(GL_LIGHTING);
        glPopMatrix();
    }

    smScene::smSceneIterator sceneIter;

    //this routine is for rendering. if you implement different objects add rendering accordingly. Viewer knows to draw
    //only current objects and their derived classes
    for (smInt sceneIndex = 0; sceneIndex < sceneList.size(); sceneIndex++)
    {
        sceneIter.setScene(sceneList[sceneIndex], this);

        //cout<<"Render:"<<sceneList[sceneIndex]->test<<endl;
        //for(smInt j=0;j<sceneList[sceneIndex]->totalObjects;j++)
        for (smInt j = sceneIter.start(); j < sceneIter.end(); j++)
        {
            //sceneObject=sceneList[sceneIndex]->sceneObjects[j];
            sceneObject = sceneIter[j];

            if (sceneObject->renderDetail.renderType & SIMMEDTK_RENDER_NONE)
            {
                continue;
            }

            glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);

            //if the custom rendering enable only render this
            if (sceneObject->renderDetail.renderType & SIMMEDTK_RENDER_CUSTOMRENDERONLY)
            {
                if (sceneObject->customRender != NULL)
                {
                    sceneObject->customRender->preDraw(sceneObject);
                    sceneObject->customRender->draw(sceneObject);
                    sceneObject->customRender->postDraw(sceneObject);
                }
            }
            else
            {
                //If there is custom renderer first render the preDraw function. which is responsible for
                //rendering before the default renderer takes place
                if (sceneObject->customRender != NULL)
                {
                    sceneObject->customRender->preDraw(sceneObject);
                }

                //drawSMStaticObject((smStaticSceneObject *)(sceneList.at(sceneIndex)->sceneObjects.at(j)));
                sceneObject->draw(p_param);


                //If there is custom renderer, render the postDraw function. which is responsible for
                //rendering after the default renderer takes place
                if (sceneObject->customRender != NULL)
                {
                    sceneObject->customRender->postDraw(sceneObject);
                }
            }

            glPopAttrib();
        }
    }
}

void setTextureMatrix()
{

    static double modelView[16];
    static double projection[16];

    // This is matrix transform every coordinate x,y,z
    // x = x* 0.5 + 0.5
    // y = y* 0.5 + 0.5
    // z = z* 0.5 + 0.5
    // Moving from unit cube [-1,1] to [0,1]
    const GLdouble bias[16] =
    {
        0.5, 0.0, 0.0, 0.0,
        0.0, 0.5, 0.0, 0.0,
        0.0, 0.0, 0.5, 0.0,
        0.5, 0.5, 0.5, 1.0
    };

    // Grab modelview and transformation matrices
    glGetDoublev(GL_MODELVIEW_MATRIX, modelView);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glActiveTextureARB(GL_TEXTURE7);
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glLoadMatrixd(bias);

    // concatating all matrice into one.
    glMultMatrixd(projection);
    glMultMatrixd(modelView);

    // Go back to normal matrix mode
    glMatrixMode(GL_MODELVIEW);
}



void smViewer::renderTextureOnView()
{

    glPushAttrib(GL_TEXTURE_BIT | GL_VIEWPORT_BIT | GL_LIGHTING_BIT);
    glUseProgramObjectARB(0);
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, 1, 20);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor4f(1, 1, 1, 1);
    glActiveTextureARB(GL_TEXTURE0);
    smTextureManager::activateTexture("depth");
    glEnable(GL_TEXTURE_2D);
    glTranslated(0, 0, -1);
    glBegin(GL_QUADS);
    glTexCoord2d(0, 0);
    glVertex3f(0, 0, 0);
    glTexCoord2d(1, 0);
    glVertex3f(1, 0, 0);
    glTexCoord2d(1, 1);
    glVertex3f(1, 1.0, 0);
    glTexCoord2d(0, 1);
    glVertex3f(0, 1.0, 0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopAttrib();
}

void smViewer::drawWithShadows(smDrawParam &p_param)
{

    smLight *light;

    for (smInt i = 0; i < lights->size(); i++)
    {
        light = lights->getByRef(i);

        if (light->castShadow)
        {
            break;
        }
    }

    renderStage = SMRENDERSTAGE_SHADOWPASS;

    glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluPerspective(light->shadorAngle, light->shadowRatio, light->shadowNearView, light->shadowFarView);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    qglviewer::Camera *cam = camera();
    qglviewer::Vec focusPosition = cam->viewDirection();
    qglviewer::Vec upVector = cam->upVector();
    light->upVector.setValue(upVector.x, upVector.y, upVector.z);
    light->direction.setValue(focusPosition.x, focusPosition.y, focusPosition.z);
    light->updateDirection();
    gluLookAt(light->lightPos.pos.x, light->lightPos.pos.y,
              light->lightPos.pos.z, focusPosition.x, focusPosition.y,
              focusPosition.z, upVector.x, upVector.y, upVector.z);
    gluLookAt(cam->position().x, cam->position().y, cam->position().z,
              cam->sceneCenter().x, cam->sceneCenter().y, cam->sceneCenter().z,
              cam->upVector().x, cam->upVector().y, cam->upVector().z);
    fbo->enable();
    smTextureManager::activateTexture("depth");
    glPushAttrib(GL_VIEWPORT_BIT | GL_ENABLE_BIT); // for GL_DRAW_BUFFER and GL_READ_BUFFER
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glClearColor(0, 0, 0, 1);
    glDepthRange(0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glViewport(0, 0, fbo->getWidth(), fbo->getHeight());
    renderScene(p_param);
    glPopAttrib();
    fbo->disable();
    setTextureMatrix();
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();

    renderStage = SMRENDERSTAGE_FINALPASS;
    renderStage = SMRENDERSTAGE_DPMAPPASS;
    glMatrixMode(GL_MODELVIEW);

    //back
    if (viewerRenderDetail & SIMMEDTK_VIEWERRENDER_DYNAMICREFLECTION)
    {
        glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluPerspective(120, camera()->aspectRatio(), camera()->zNear(), camera()->zFar());
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        gluLookAt(-35, 38.666, 19.7, 90.8, 10.25, 200.52, -0.1239, 0.955, -0.269577); ///perspective 120 cok iyi
        backfbo->enable();
        glPushAttrib(GL_VIEWPORT_BIT | GL_ENABLE_BIT); // for GL_DRAW_BUFFER and GL_READ_BUFFER
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glClearColor(0, 0, 0, 1);
        glDepthRange(0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glViewport(0, 0, backfbo->getWidth(), backfbo->getHeight());
        renderScene(p_param);
        glPopAttrib();
        backfbo->disable();
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glPopAttrib();
    }

    renderStage = SMRENDERSTAGE_FINALPASS;
    glMatrixMode(GL_MODELVIEW);

    if (renderandreflection != NULL && viewerRenderDetail & SIMMEDTK_VIEWERRENDER_DYNAMICREFLECTION)
    {
        renderandreflection->switchEnable();
    }

    {
        //why is this scoped?
        glDisable(GL_CULL_FACE);
        renderScene(p_param);
    }

    if (renderandreflection != NULL && viewerRenderDetail & SIMMEDTK_VIEWERRENDER_DYNAMICREFLECTION)
    {
        renderandreflection->switchDisable();
    }

}


inline void smViewer::setToDefaults()
{

    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, (smGLReal*)defaultDiffuseColor.toGLColor());
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, (smGLReal*)defaultSpecularColor.toGLColor());
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, (smGLReal*)defaultAmbientColor.toGLColor());
    glColor4fv(defaultDiffuseColor.toGLColor());
}

inline void smViewer::adjustFPS()
{

    static smInt _unlimitedFPSVariableChanged = 0;
    smInt unlimitedFPSVariableChangedCurrent;
    unlimitedFPSVariableChangedCurrent = unlimitedFPSVariableChanged;

    if (_unlimitedFPSVariableChanged < unlimitedFPSVariableChangedCurrent)
    {
        _unlimitedFPSVariableChanged = unlimitedFPSVariableChangedCurrent;

        if (unlimitedFPSEnabled)
        {
            SetVSync(false);
        }
        else
        {
            SetVSync(true);
        }
    }
}

smBool  smViewer::checkCameraCollisionWithScene()
{

    static bool collided = false;
    static bool prev_collided = false;
    static smVec3f  lastCollidedHatpicPos(deviceCameraPos.x, deviceCameraPos.y, deviceCameraPos.z);
    static smVec3f  proxy_hapticPos(deviceCameraPos.x, deviceCameraPos.y, deviceCameraPos.z);
    static float radiusEffective = 0;
    static smVec3f  last_collisionNormal;
    static smVec3f prevPosition(deviceCameraPos.x, deviceCameraPos.y, deviceCameraPos.z);
    static smFloat radiusMotion = 0;

    smVec3f collisionNormal(0, 0, 0);
    collided = false;
    radiusEffective = 0;
    smVec3f hPos(deviceCameraPos.x, deviceCameraPos.y, deviceCameraPos.z);
    radiusMotion = hPos.distance(prevPosition);
    prevPosition = hPos;

    for (int i = 0; i < collisionMeshes.size(); i++)
    {
        float distance;
        smMesh*mesh = collisionMeshes[i];
        int nbrVert = mesh->nbrVertices;

        for (int j = 0; j < nbrVert; j++)
        {
            distance = mesh->vertices[j].distance(hPos);

            if (prev_collided)
            {
                smVec3f  posVector = (hPos - lastCollidedHatpicPos);
                float distance = posVector.module();
                posVector.normalize();

                if (posVector.dot(last_collisionNormal) < 0)
                {
                    if ((distance - cameraRadius) > 0)
                    {
                        radiusEffective = distance - cameraRadius;
                    }

                    if (radiusEffective < 0)
                    {
                        radiusEffective = 0;
                    }

                    collided = true;
                    break;
                }
            }
            else
            {
                radiusEffective = 0;
            }

            if (distance < cameraRadius + radiusEffective + radiusMotion)
            {
                collisionNormal = collisionNormal + mesh->vertNormals[j];

                if (!prev_collided)
                {
                    lastCollidedHatpicPos = hPos;
                    last_collisionNormal = collisionNormal;
                }

                collided = true;
                prev_collided = true;
            }
        }
    }

    if (collided)
    {
        proxy_hapticPos = lastCollidedHatpicPos;
        last_collisionNormal.normalize();
    }
    else
    {
        proxy_hapticPos = hPos;
        prev_collided = false;
    }

    return collided;
}

void smViewer::addCollisionCheckMeshes(smMesh *p_mesh)
{

    collisionMeshes.push_back(p_mesh);
}


///main drawing routine for Rendering of all objects in the scene
void smViewer::draw()
{

    static smDrawParam param;
    static QString fps("FPS: %1");
    static QFont font;
    static smQuatd quat;

    if (viewerRenderDetail & SIMMEDTK_VIEWERRENDER_DISABLE)
    {
        return;
    }

    font.setPixelSize(10);
    param.rendererObject = this;
    param.caller = this;
    param.data = NULL;

    beginModule();

    adjustFPS();

    if (viewerRenderDetail & SIMMEDTK_VIEWERRENDER_FULLSCREEN)
    {
        glViewport(0, 0, screenResolutionWidth, screenResolutionHeight);
    }

    glClearColor(0.05f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    enableLights();

    glUseProgramObjectARB(0);
    glDisable(GL_VERTEX_PROGRAM_ARB);
    glDisable(GL_FRAGMENT_PROGRAM_ARB);

    if (viewerRenderDetail & SIMMEDTK_VIEWERRENDER_FADEBACKGROUND)
    {
        smGLUtils::fadeBackgroundDraw();
    }

    setToDefaults();

    for (smInt i = 0; i < objectList.size(); i++)
    {
        if (objectList[i]->drawOrder == SIMMEDTK_DRAW_BEFOREOBJECTS);

        objectList[i]->draw(param);
    }

    if (viewerRenderDetail & SIMMEDTK_VIEWERRENDER_SOFTSHADOWS)
    {
        drawWithShadows(param);
    }
    else
    {
        glDisable(GL_CULL_FACE);
        renderScene(param);
    }

    //for font display
    setToDefaults();

    for (smInt i = 0; i < objectList.size(); i++)
    {
        if (objectList[i]->drawOrder == SIMMEDTK_DRAW_AFTEROBJECTS);

        objectList[i]->draw(param);
    }

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glUseProgramObjectARB(0);
    glDisable(GL_VERTEX_PROGRAM_ARB);
    glDisable(GL_FRAGMENT_PROGRAM_ARB);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    windowOutput->updateText("VFPS", fps.arg(currentFPS()));
    glPopAttrib();

    endModule();
}

///called by the module before each frame starts
void smViewer::beginFrame()
{

    if (terminateExecution == true)
    {
        terminationCompleted = true;
        this->close();
    }

    makeCurrent();
}

///called by the module after each frame ends
void smViewer::endFrame()
{

    swapBuffers();
}



void smViewer::keyPressEvent(QKeyEvent *e)
{

    smEvent *eventKeyboard;
    smLight *light, *light1;

    eventKeyboard = new smEvent();
    eventKeyboard->eventType = SIMMEDTK_EVENTTYPE_KEYBOARD;
    eventKeyboard->senderId = this->getModuleId();
    eventKeyboard->senderType = SIMMEDTK_SENDERTYPE_MODULE;
    eventKeyboard->data = new smKeyboardEventData();
    ((smKeyboardEventData*)eventKeyboard->data)->keyBoardKey = e->key();

    if (eventDispatcher)
    {
        eventDispatcher->sendEventAndDelete(eventKeyboard);
    }

    if (e->key() == Qt::Key_C)
    {
        enableCameraMotion = !enableCameraMotion;
    }

    if (e->key() == Qt::Key_T)
    {
        checkCameraCollision = !checkCameraCollision;
        cout << "checkCameraCollision" << checkCameraCollision << endl;
    }

    if (e->key() == Qt::Key_W)
    {
        this->viewerRenderDetail = this->viewerRenderDetail & (~(this->viewerRenderDetail & SIMMEDTK_VIEWERRENDER_WIREFRAMEALL));
    }

    if (e->key() == Qt::Key_Escape)
    {
        smSDK::getInstance()->shutDown();
        this->close();
    }
}

void smViewer::addObject(smCoreClass *object)
{

    smSDK::addRef(object);
    objectList.push_back(object);
}

void smViewer::handleEvent(smEvent *p_event)
{

    smHapticOutEventData *hapticEventData;
    smHapticInEventData *hapticInEventData;
    smCameraEventData *cameraData;
    smLightMotionEventData *lightPosData;
    smLight *light;
    smVec3<smDouble> lightDir;
    smVec3<smDouble> lightUp;
    smVec3<smDouble> transverseDir;

    switch (p_event->eventType.eventTypeCode)
    {
    case SIMMEDTK_EVENTTYPE_HAPTICOUT:
        //left here as an example for implementation
        //hapticEventData=(smHapticOutEventData *)p_event->data;
        break;

    case SIMMEDTK_EVENTTYPE_HAPTICIN:
        //left here as an example for implementation
        //hapticInEventData=(smHapticInEventData *)p_event->data;
        break;

    case SIMMEDTK_EVENTTYPE_CAMERA_UPDATE:
        cameraData = (smCameraEventData *)p_event->data;
        deviceCameraPos = cameraData->pos;
        deviceCameraDir = cameraData->direction;
        deviceCameraUpDir = cameraData->upDirection;
        break;

    case SIMMEDTK_EVENTTYPE_LIGHTPOS_UPDATE:
        lightPosData = (smLightMotionEventData*)p_event->data;

        if (lights->size() < lightPosData->lightIndex)
        {
            light = lights->getByRef(lightPosData->lightIndex);
            light->lightPos.pos = lightPosData->pos;
            light->direction = lightPosData->direction;
        }

        break;
    }
}

void smViewer::addText(QString p_tag)
{

    windowOutput->addText(p_tag, QString(""));
}

void smViewer::updateText(QString p_tag, QString p_string)
{

    windowOutput->updateText(p_tag, p_string);
}
void smViewer::updateText(smInt p_handle, QString p_string)
{

    windowOutput->updateText(p_handle, p_string);
}


void smViewer::dropEvent(QDropEvent *event)
{

    QString text;
    QList<QUrl> urls = event->mimeData()->urls();

    if (urls.isEmpty())
    {
        return;
    }

    QString fileName = urls.first().toLocalFile();

    cout << fileName.toAscii().data() << endl;
}


void smViewer::dragEnterEvent(QDragEnterEvent *event)
{

    if (event->mimeData()->hasFormat("text/uri-list"))
    {
        printf("Dra accepted --------\n");
        event->acceptProposedAction();
    }
}

void smViewer::exec()
{

    show();

    for (smInt i = 0; i < forms.size(); i++)
    {
        forms[i]->show();
    }
}

void smViewer::addForm(QDialog *p_form)
{

    forms.push_back(p_form);
}
