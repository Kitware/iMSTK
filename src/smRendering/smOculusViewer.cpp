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
// Authors: Sean Radigan
//
// Contact:
//---------------------------------------------------------------------------

#include <iostream>

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "smRendering/smOculusViewer.h"
#include "smRendering/smGLRenderer.h"

#ifdef _WIN32 || WIN32
    #define OVR_OS_WIN32
#elif defined(__APPLE__)
    #define OVR_OS_MAC
#else
    #define OVR_OS_LINUX
    #include <X11/Xlib.h>
    #include <GL/glx.h>
#endif

#include <OVR_CAPI_GL.h>

/// \brief Calculate the next power of two
///
/// \details Code from:
/// http://nuclear.mutantstargoat.com/hg/oculus2/file/tip
///
/// \return Returns the next power of two
static unsigned int next_pow2(unsigned int x)
{
    x -= 1;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x + 1;
}

smOculusViewer::smOculusViewer() : smViewer()
{
    hmd = nullptr;
    fbWidth = 0;
    fbHeight = 0;
    fbTexWidth = 0;
    fbTexHeight = 0;
    fbTex = 0;
    fbDepth = 0;
    distortionCaps = 0;
    hmdCaps = 0;
    oculusFBO = 0;
}

smOculusViewer::~smOculusViewer()
{

}

void smOculusViewer::init()
{
    static smDrawParam param;

    if (isInitialized)
    {
        return;
    }

    param.rendererObject = this;
    param.caller = this;
    param.data = NULL;

    ovr_Initialize();
    this->initGLContext();
    this->initGLCaps();
    this->initObjects(param);
    this->initResources(param);
    this->initScenes(param);
    if (-1 == this->initOculus())
    {
        isInitialized = false;
        //probably call clean up?
        ovr_Shutdown();
    }
    else
    {
        isInitialized = true;
    }
}

void smOculusViewer::cleanUp()
{
    destroyFboListItems();
    destroyGLContext();
    cleanupOculus();
    ovr_Shutdown();
}

void smOculusViewer::beginFrame()
{
    if (terminateExecution == true)
    {
        terminationCompleted = true;
    }
}

void smOculusViewer::endFrame()
{
    //This is here to override swapping buffers,
    // Oculus doesn't like it when you swap buffers
}

void smOculusViewer::renderToScreen(const smRenderOperation &p_rop, smDrawParam p_param)
{
    int i;
    ovrMatrix4f proj;
    ovrPosef pose[2];
    ovrTrackingState ts;

    //the drawing starts with a call to ovrHmd_BeginFrame
    ovrHmd_BeginFrame(hmd, 0);

    //start drawing onto our texture render target
    glBindFramebuffer(GL_FRAMEBUFFER, oculusFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Enable lights
    p_rop.scene->enableLights();

    ovrHmd_GetEyePoses(hmd, 0, &(eyeRdesc->HmdToEyeViewOffset), pose, &ts);
    //for each eye ...
    for (i = 0; i < 2; i++) {
        ovrEyeType eye = hmd->EyeRenderOrder[i];
        glm::mat4 glmProj;
        glm::mat4 glmView;
        glm::mat4 tmpMat;
        glm::vec3 tmpVec;
        glm::quat tmpQuat;
        // -- viewport transformation --
        //setup the viewport to draw in the left half of the framebuffer when
        // we're rendering the left eye's view (0, 0, width/2, height), and
        // in the right half of the framebuffer for the right eye's view
        // (width/2, 0, width/2, height)
        glViewport(eye == ovrEye_Left ? 0 : (fbWidth / 2),
                   0, (fbWidth / 2), fbHeight);

        processViewerOptions();

        //-- projection transformation --
        //we'll just have to use the projection matrix supplied by the oculus
        // SDK for this eye note that libovr matrices are the transpose of what
        // OpenGL expects, so we have to transpose them.
        proj = ovrMatrix4f_Projection(hmd->DefaultEyeFov[eye], 0.1, 500.0, 1);
        //copy the ovr matrix into a glm matrix and transpose it
        memcpy(glm::value_ptr(tmpMat), proj.M, sizeof(glm::mat4));
        glmProj = glm::transpose(tmpMat);
        p_param.projMatrix = glm::value_ptr(glmProj);

        // -- view/camera transformation --
        //we need to construct a view matrix by combining all the information
        // provided by the oculus SDK, about the position and orientation of
        // the user's head in the world.
        glmView = p_rop.scene->camera.view;

        //retrieve the orientation quaternion and
        // convert it to a rotation matrix
        tmpQuat = glm::quat(-pose[eye].Orientation.w,
                            pose[eye].Orientation.x,
                            pose[eye].Orientation.y,
                            pose[eye].Orientation.z);
        glmView = glm::toMat4(tmpQuat) * glmView;

        //translate the view matrix with the positional tracking
        if(ts.StatusFlags &
           (ovrStatus_OrientationTracked | ovrStatus_PositionTracked))
        {
            //Not sure why these values have to be negated...but they do
            tmpVec = glm::vec3(-ts.HeadPose.ThePose.Position.x,
                               -ts.HeadPose.ThePose.Position.y,
                               -ts.HeadPose.ThePose.Position.z);
            glmView = glm::translate(glmView, tmpVec);
        }
        //move the camera to the eye level of the user from Oculus SDK settings
        tmpVec = glm::vec3(0, -ovrHmd_GetFloat(hmd, OVR_KEY_EYE_HEIGHT, 1.65), 0);
        glmView = glm::translate(glmView, tmpVec);

        //Load the matrix reference into the draw parameters
        p_param.viewMatrix = glm::value_ptr(glmView);

        //Render Scene
        smGLRenderer::renderScene(p_rop.scene, p_param);
    }
    //after drawing both eyes into the texture render target, revert to
    // drawing directly to the display, and we call ovrHmd_EndFrame, to let the
    // Oculus SDK draw both images properly compensated for lens distortion and
    // chromatic abberation onto the HMD screen.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    ovrHmd_EndFrame(hmd, pose, &fbOvrTex[0].Texture);

    //workaround for the oculus sdk distortion renderer bug, which uses a
    // shader program, and doesn't restore the original binding when it's done.
    glUseProgram(0);
}

int smOculusViewer::initOculus(void)
{
    int i, x, y;
    unsigned int flags;

    if (!(hmd = ovrHmd_Create(0))) {
        std::cerr << "Failed to open Oculus HMD, falling back to virtual debug HMD\n";
        if (!(hmd = ovrHmd_CreateDebug(ovrHmd_DK2))) {
            std::cerr << "Failed to create virtual debug HMD\n";
            return -1;
        }
    }
    std::cout << "Initialized HMD: "
              << hmd->Manufacturer << " - " << hmd->ProductName << "\n";

    //enable position and rotation tracking
    ovrHmd_ConfigureTracking(hmd,
                             (ovrTrackingCap_Orientation |
                              ovrTrackingCap_MagYawCorrection |
                              ovrTrackingCap_Position),
                             0);
    //retrieve the optimal render target resolution for each eye
    eyeRes[0] = ovrHmd_GetFovTextureSize(hmd, ovrEye_Left,
                                         hmd->DefaultEyeFov[0], 1.0);
    eyeRes[1] = ovrHmd_GetFovTextureSize(hmd, ovrEye_Right,
                                         hmd->DefaultEyeFov[1], 1.0);

    //and create a single render target texture to encompass both eyes
    fbWidth = eyeRes[0].w + eyeRes[1].w;
    fbHeight = eyeRes[0].h > eyeRes[1].h ? eyeRes[0].h : eyeRes[1].h;
    updateRenTarg(fbWidth, fbHeight);

    //fill in the ovrGLTexture structures that describe our
    // render target texture
    for (i = 0; i < 2; i++) {
        fbOvrTex[i].OGL.Header.API = ovrRenderAPI_OpenGL;
        fbOvrTex[i].OGL.Header.TextureSize.w = fbTexWidth;
        fbOvrTex[i].OGL.Header.TextureSize.h = fbTexHeight;
        //this next field is the only one that differs between the two eyes
        fbOvrTex[i].OGL.Header.RenderViewport.Pos.x = i == 0 ? 0 : fbWidth / 2.0;
        fbOvrTex[i].OGL.Header.RenderViewport.Pos.y = 0;
        fbOvrTex[i].OGL.Header.RenderViewport.Size.w = fbWidth / 2.0;
        fbOvrTex[i].OGL.Header.RenderViewport.Size.h = fbHeight;
        fbOvrTex[i].OGL.TexId = fbTex;//both eyes will use the same texture id
    }

    //fill in the ovrGLConfig structure needed by the SDK to draw our
    // stereo pair to the actual HMD display (SDK-distortion mode)
    memset(&glCfg, 0, sizeof glCfg);
    glCfg.OGL.Header.API = ovrRenderAPI_OpenGL;
    glCfg.OGL.Header.BackBufferSize = hmd->Resolution;
    glCfg.OGL.Header.Multisample = 1;

#if defined(OVR_OS_WIN32)
    glCfg.OGL.Window = sfmlWindow->getSystemHandle();
    glCfg.OGL.DC = wglGetCurrentDC();
#elif defined(OVR_OS_LINUX)
    glCfg.OGL.Disp = glXGetCurrentDisplay();
#endif 


    if (hmd->HmdCaps & ovrHmdCap_ExtendDesktop) {
        std::cout << "running in \"extended desktop\" mode\n";
    }
    else {
        //to sucessfully draw to the HMD display in "direct-hmd" mode, we
        // have to call ovrHmd_AttachToWindow
        // XXX: this doesn't work properly yet due to bugs in the oculus
        // 0.4.1 sdk/driver
#ifdef WIN32
        ovrHmd_AttachToWindow(hmd, glCfg.OGL.Window, nullptr, nullptr);
#else
        ovrHmd_AttachToWindow(hmd, (void*)glXGetCurrentDrawable(), nullptr, nullptr);
#endif
        std::cout << "running in \"direct-hmd\" mode\n";
    }

    //enable low-persistence display and dynamic prediction for
    // lattency compensation
    hmdCaps = ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction;
    ovrHmd_SetEnabledCaps(hmd, hmdCaps);

    //configure SDK-rendering and enable chromatic abberation correction,
    // vignetting, and timewrap, which shifts the image before drawing to
    // counter any lattency between the call to ovrHmd_GetEyePose and
    // ovrHmd_EndFrame.
    distortionCaps = ovrDistortionCap_Vignette |
                     ovrDistortionCap_TimeWarp | ovrDistortionCap_Overdrive;
    if (!ovrHmd_ConfigureRendering(hmd, &glCfg.Config, distortionCaps,
                                   hmd->DefaultEyeFov, eyeRdesc)) {
        std::cerr << "failed to configure distortion renderer\n";
    }

    //disable the "health and safety warning"
    //ovrhmd_EnableHSWDisplaySDKRender(hmd, 0);
    ovrHmd_DismissHSWDisplay(hmd);
    return 0;
}

void smOculusViewer::cleanupOculus(void)
{
    if (hmd) {
        ovrHmd_Destroy(hmd);
    }
}

void smOculusViewer::updateRenTarg(int width, int height)
{
    if (!oculusFBO) {
        //if oculusFBO does not exist,
        // then nothing does...create every opengl object
        glGenFramebuffers(1, &oculusFBO);
        glGenTextures(1, &fbTex);
        glGenRenderbuffers(1, &fbDepth);

        glBindTexture(GL_TEXTURE_2D, fbTex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, oculusFBO);

    fbTexWidth = next_pow2(width);  //calculate the next power of two in both
    fbTexHeight = next_pow2(height);// dimensions, use that as a texture size

    //create and attach the texture that will be used as a color buffer
    glBindTexture(GL_TEXTURE_2D, fbTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fbTexWidth, fbTexHeight, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, fbTex, 0);

    //create and attach the renderbuffer that will serve as our z-buffer
    glBindRenderbuffer(GL_RENDERBUFFER, fbDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
                          fbTexWidth, fbTexHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, fbDepth);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "incomplete framebuffer!\n";
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    std::cout << "created render target: " << width << "x" << height
              << " (texture size: " << fbTexWidth << "x" << fbTexHeight << ")\n";
}
