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

#ifndef SMOCULUSVIEWER_H
#define SMOCULUSVIEWER_H

// SimMedTK includes
#include "Rendering/OpenGLViewer.h"

// OculusSDK includes
#include <OVR_CAPI_GL.h>

/// \brief This viewer class allows content to be rendered to an Oculus Rift
///
class OculusViewer : public OpenGLViewer
{
public:
    OculusViewer();
    virtual ~OculusViewer();
    virtual bool init() override;
protected:
    virtual void cleanUp() override;
    virtual void beginFrame() override;
    virtual void endFrame() override;
    virtual void renderToScreen(const RenderOperation &p_rop) override;
private:
    ovrBool ovrInitialized;
    ovrHmd hmd;
    ovrSizei eyeRes[2];
    ovrEyeRenderDesc eyeRdesc[2];
    int fbWidth, fbHeight;
    int fbTexWidth, fbTexHeight;
    ovrGLTexture fbOvrTex[2];
    union ovrGLConfig glCfg;
    unsigned int distortionCaps;
    unsigned int hmdCaps;
    unsigned int oculusFBO, fbTex, fbDepth;

    /// \brief Initializes the HMD and its render capabilities and information
    ///
    /// \details Code adapted from: http://nuclear.mutantstargoat.com/hg/oculus2/file/tip
    ///
    /// \return Returns -1 on error and 0 on success
    int initOculus(void);

    /// \brief Cleans up Oculus resources
    ///
    void cleanupOculus(void);

    /// \brief Sets up the Oculus framebuffers and textures
    ///
    /// \details Code adapted from: http://nuclear.mutantstargoat.com/hg/oculus2/file/tip
    void updateRenTarg(int width, int height);
};

#endif
